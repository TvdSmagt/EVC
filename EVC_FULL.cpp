//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <vector>
#include <raspicam/raspicam_cv.h>
//C
#include <stdio.h>
//C++
#include <iostream>
#include <sstream>
//local
#include <wiringSerial.h>

using namespace cv;
using namespace std;
// Global variables
enum DriveCommand {DRIVE_STRAIGHT=0,TURN_LEFT,TURN_RIGHT};
enum SignDetected {SIGN_NONE=0,SIGN_LEFT,SIGN_RIGHT, SIGN_STRAIGHT, SIGN_STOP, SIGN_U_TURN, SIGN_UNKNOWN};
cv::Mat src,dst,src_crop, dst_color, gray, thresh , hough;
cv::Mat src_path,dst_path,src_sign,dst_sign;
float pctCropHeight = 0.0; //0.1
int iThresh = 60;
int maxLines = 25;
int curved = 1, straight = 1;
int ITER;
int FrameSkip = 5;
vector<Vec4i> lines;
double dWidth,dHeight;
char keyboard; //input from keyboard
bool video = true;

//Functions
cv::Mat makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, int nRows);
void processVideo(char* videoFilename);
int findPath(InputArray src, OutputArray dst, bool display);
int findSign(InputArray src, OutputArray dst, bool display);
void processFrame(InputArray src,OutputArray dst,OutputArray hough, double dWidth, double dHeight);
void processFrame_adaptive(InputArray src,OutputArray dst,OutputArray hough, double dWidth, double dHeight);
int searchLongestLine_Angle(InputArray src,OutputArray dst, double dWidth, double dHeight);
int searchLongestLine_Straight(InputArray src,OutputArray dst, double dWidth, double dHeight);
void help();
void goLeft(double degrees);
void goForward();
void goRight(double degrees);
void ArduinoCommand(int fd, int command);

void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program is used for the Embedded Visual Control Project for Group 7."  << endl                                                                                   				     << endl
    << "Usage:"                                                                     << endl
    << "./bg_sub {-vid <video filename>}"                     			    << endl
    << "for example: ./bg_sub -vid video.avi"                                       << endl
    << "--------------------------------------------------------------------------" << endl
    << endl;
}
int main(int argc, char* argv[])
{
    //print help information
    help();
    //check for the input parameter correctness
    if(argc != 2) {
        cerr <<"Incorrect input list" << endl;
        cerr <<"exiting..." << endl;
        return EXIT_FAILURE;
    }
    else {
    	processVideo(argv[1]);
    }
    //destroy GUI windows
    destroyAllWindows();
    return EXIT_SUCCESS;
}
void processVideo(char* videoFilename) {
	//Open Serial Connection With Arduino
	int fd = serialOpen ("/dev/ttyACM0",9600);
	if (fd<0){exit(EXIT_FAILURE);}
	serialFlush(fd);

    //create the capture object
	if (video){
    VideoCapture capture(videoFilename);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }} else {
	raspicam::RaspiCam_Cv capture;
	capture.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
	if (!capture.open()) {cerr<<"Error opening the capture"<<endl;exit(EXIT_FAILURE);}}
	
    //Get screen sizes
    dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
    //Set Cropping parameters
    float yFl = float(dHeight) * pctCropHeight;
    int y = int(yFl + 0.5);
    cv::Rect myROI(0,y,dWidth,dHeight-y);

    //read input data. ESC or 'q' for quitting
    keyboard = 0;
    ITER = 0;
    while( keyboard != 'q' && keyboard != 27 ){
		double time_ = cv::getTickCount();
		if (video){
		    //read the current frame
		    if(! capture.grab()){
		        cerr << "Unable to read next frame." << endl;
		        cerr << "Exiting..." << endl;
		        exit(EXIT_FAILURE);
        }}else{
			capture.retrieve (src);
			if(!capture.read(src)) {
				    cerr << "Unable to read next frame." << endl;
				    cerr << "Exiting..." << endl;
				    exit(EXIT_FAILURE);
				}}
        //Start processing frame
        if (ITER % FrameSkip==FrameSkip/5 && !src.empty() && ITER > 20){
			cout << "\n" << ITER << " processing image...";
			//Crop image to remove top part which is not of interest
			//src_crop = src(myROI);
			src_path = src(myROI);
			/*processFrame(src_path, thresh, hough, dWidth, dHeight); 
			cout << dWidth << " " << dHeight;
			imshow("Result",hough);
			int iDirPath = 0; */
			int iDirPath = findPath(src_path,dst_path,false);
			int iDirSign = findSign(src_sign,dst_sign,false);

	
        //Give commands
			switch (iDirPath){
			case TURN_LEFT :{
				ArduinoCommand(fd,30);
				goLeft(20);
				//putText(src,"Go Left",Point2f(0,dHeight*0.5), FONT_HERSHEY_PLAIN, 20,  Scalar(255,255,255));
				break;
			}
			case TURN_RIGHT:{
				ArduinoCommand(fd,60);
				goRight(20);
				//putText(src,"Go Right",Point2f(0,dHeight*0.5), FONT_HERSHEY_PLAIN, 20,  Scalar(255,255,255));
				break;
			}
			case DRIVE_STRAIGHT:{
				ArduinoCommand(fd,9);
				goForward();
				//putText(src,"Go Straight",Point2f(0,dHeight*0.5), FONT_HERSHEY_PLAIN, 20,  Scalar(255,255,255));
				break;
			}
			}
	    	//namedWindow("Result", WINDOW_NORMAL);
	    	//cv::resizeWindow("Result", dWidth, dHeight);
	    	//imshow("Result",src);
		double secondsElapsed = double (cv::getTickCount()-time_)/double(cv::getTickFrequency());
		cout << "Time :" << secondsElapsed << " FPS: " << 1/secondsElapsed;
        }
        //Finalize
        ITER++;
        keyboard = (char)waitKey( 30 );
    }
    cout<<"Stop capture..."<<endl;
    capture.release();
    //delete capture object
    //capture.release();
}

int findPath(InputArray src, OutputArray dst, bool display){
	vector<Mat> Images;

	//Histogram normalization.equalization?
	//Background subtraction?
	//Gaussian blur?

	processFrame(src,thresh,hough,dWidth,dHeight);
	//processFrame_adaptive(src_crop,thresh,hough,dWidth,dHeight);

	//Process results into a working roadmap
	int a_c = -1, a_s = -1;
	if (curved){
		a_c = searchLongestLine_Angle(hough,hough,dWidth,dHeight);
	}
	if (straight){
		a_s = searchLongestLine_Straight(hough,hough,dWidth,dHeight);
	}
	//Calculate direction
	double d_c, d_s;
	d_c = a_c - maxLines/2;
	d_s = a_s - maxLines/2;

	Mat command(dHeight,dWidth,CV_8UC3,Scalar(0,0,0));
	int direction = 0;
	if (curved && straight){
		if(abs(d_s)<maxLines/8){
			direction = DRIVE_STRAIGHT;
		}else if(d_c>-maxLines/8){
			direction = TURN_LEFT;
		}else if(d_c<maxLines/8){
			direction = TURN_RIGHT;
		}
	}
	else if (curved){
		if(d_c>-maxLines/8){
			direction = TURN_LEFT;
		}else if(d_c<maxLines/8){
			direction = TURN_RIGHT;
		}else{
			direction = DRIVE_STRAIGHT;
		}
	}
	else if (straight){
		if(d_s<-maxLines/8){
			direction = TURN_LEFT;
		}else if(d_s>maxLines/8){
			direction = TURN_RIGHT;
		}else{
			direction = DRIVE_STRAIGHT;
		}
	}
	hough.copyTo(dst);
	if (display){
	//Display different versions
	Mat srcc;
	src.copyTo(srcc);
	Images.push_back(srcc);
	Images.push_back(thresh);
	Images.push_back(hough);
	Images.push_back(command);
	Mat canvas = makeCanvas(Images, dHeight,2);
	namedWindow("Canvas", WINDOW_NORMAL);
	cv::resizeWindow("Canvas", dWidth/2, dHeight/2);
	imshow("Canvas",canvas);
	}
	return direction;
}

int findSign(InputArray src, OutputArray dst, bool display){

	return 0;
}
void processFrame(InputArray src,OutputArray thresh,OutputArray hough, double dWidth, double dHeight) {
	//Apply Threshold
	Mat gray;
        Mat canvas(dHeight,dWidth,CV_8UC3,Scalar(0,0,0));
	cvtColor(src,gray,CV_RGB2GRAY);
	threshold(gray,thresh,iThresh,255,THRESH_BINARY_INV);
	HoughLinesP( thresh, lines, 1, CV_PI/180, 200, 250, 0);
	for( size_t i = 0; i < lines.size(); i++ )
	{
		line(canvas, Point(lines[i][0], lines[i][1]),
			Point(lines[i][2], lines[i][3]), Scalar(255,255,255), 8, 3 );
	}
	//gray.copyTo(thresh);
	//gray.copyTo(hough);
	canvas.copyTo(hough);
}
void processFrame_adaptive(InputArray src,OutputArray dst,OutputArray hough, double dWidth, double dHeight){
	Mat gray;
	Mat canvas(dHeight,dWidth,CV_8UC3,Scalar(0,0,0));
	cvtColor(src,gray,CV_RGB2GRAY);
	adaptiveThreshold(gray,dst,255,ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV,55,20);
	//adaptiveThreshold(gray,dst2,255,ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV,85,20);
	//Apply HoughLines
	HoughLinesP( dst, lines, 1, CV_PI/180, 200, 100, 0);
	for( size_t i = 0; i < lines.size(); i++ )
	{
		line(canvas, Point(lines[i][0], lines[i][1]),
			Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 2, 3 );
	}
	canvas.copyTo(hough);
}
int searchLongestLine_Straight(InputArray src,OutputArray dst, double dWidth, double dHeight){
	vector<int> max_angle,max_range;
	Mat temp;
	src.copyTo(temp);
	Rect bounds(0, 0, dWidth, dHeight);
	for(int i = 2; i<=maxLines-2; i++)
	{
		Point cen(int(dWidth*i/maxLines),int(dHeight));
		for (int r = 5; r <= dHeight+1; r++)
		{
			Point p2(dWidth*i/maxLines,dHeight-r);
			if (!bounds.contains(p2) || (temp.at<Vec3b>(p2).val[2] >0)){
				//line(hough, cen, p2, Scalar(0,255,0), 2, 3 );
				max_angle.push_back(i);
				max_range.push_back(r);
				break;
			}
		}
	}
	int x_max, r = 0;
		for (uint64 i = 0;i<max_range.size();i++){
			if ((r < max_range[i]) || ((r <= max_range[i])&&(abs(max_angle[i]-maxLines/2) < abs(x_max-maxLines/2)))){
				x_max = max_angle[i];
				r = max_range[i];
			}
		}
	Point cen(int(dWidth*x_max/maxLines),int(dHeight));
	Point p2(int(dWidth*x_max/maxLines),int(dHeight-r));
	line(temp, cen, p2 , Scalar(255,0,0), 8, 3 );
	temp.copyTo(dst);
	return x_max;
}
int searchLongestLine_Angle(InputArray src,OutputArray dst, double dWidth, double dHeight){
	vector<int> max_angle,max_range;
	Mat temp;
	double a;
	src.copyTo(temp);
	Point cen(int(dWidth/2),int(dHeight));
	Rect bounds(0, 0, dWidth, dHeight);
	for(int i = 2; i<=maxLines; i++)
	{
		a = (i)*CV_PI/(maxLines+2) - 0.5 * CV_PI;
		for (int r = 5; r <= 2000; r++)
		{
			Point p2(cen.x-sin(a)*r, cen.y-cos(a)*r);
			if (!bounds.contains(p2) || (temp.at<Vec3b>(p2).val[2] >0)){
				//line(hough, cen, p2, Scalar(0,255,0), 2, 3 );
				max_angle.push_back(i);
				max_range.push_back(r);
				break;
			}
		}
	}
	int angle, r = 0;
	for (uint64 i = 0;i<max_range.size();i++){
		if ((r < max_range[i]) || ((r <= max_range[i])&&(abs(max_angle[i]-maxLines/2) < abs(angle-maxLines/2)))){
			angle = max_angle[i];
			r = max_range[i];
		}
	}
	a = (angle)*CV_PI/(maxLines+2) - 0.5 * CV_PI;
	Point p2(cen.x-sin(a)*r, cen.y-cos(a)*r);
	line(temp, cen, p2 , Scalar(255,0,0), 8, 3 );
	temp.copyTo(dst);
	return angle;
}

void goLeft(double degrees){
	//Call Arduino command: TurnLeft(double degrees)
	cout << "Turn Left\n";
}
void goForward(){
	//Call Arduino command: GoStraight(double cm)
	cout << "Go Forward\n";
}
void goRight(double degrees){
	//Call Arduino command: TurnRight(double degrees)
	cout << "Turn Right\n";
}
void ArduinoCommand(int fd, int command){
	for(int i = 0;i<4;i++){
	serialPutchar(fd,command);
	}
	for(int j = 0;j<4;j++){
	int answer = serialGetchar(fd);
	cout << answer << "\n";
	}
}

cv::Mat makeCanvas(std::vector<cv::Mat>& vecMat, int windowHeight, int nRows) {
        int N = vecMat.size();
        nRows  = nRows > N ? N : nRows;
        int edgeThickness = 10;
        int imagesPerRow = ceil(double(N) / nRows);
        int resizeHeight = floor(2.0 * ((floor(double(windowHeight - edgeThickness) / nRows)) / 2.0)) - edgeThickness;
        int maxRowLength = 0;

        std::vector<int> resizeWidth;
        for (int i = 0; i < N;) {
                int thisRowLen = 0;
                for (int k = 0; k < imagesPerRow; k++) {
                        double aspectRatio = double(vecMat[i].cols) / vecMat[i].rows;
                        int temp = int( ceil(resizeHeight * aspectRatio));
                        resizeWidth.push_back(temp);
                        thisRowLen += temp;
                        if (++i == N) break;
                }
                if ((thisRowLen + edgeThickness * (imagesPerRow + 1)) > maxRowLength) {
                        maxRowLength = thisRowLen + edgeThickness * (imagesPerRow + 1);
                }
        }
        int windowWidth = maxRowLength;
        cv::Mat canvasImage(windowHeight, windowWidth, CV_8UC3, Scalar(0, 0, 0));

        for (int k = 0, i = 0; i < nRows; i++) {
                int y = i * resizeHeight + (i + 1) * edgeThickness;
                int x_end = edgeThickness;
                for (int j = 0; j < imagesPerRow && k < N; k++, j++) {
                        int x = x_end;
                        cv::Rect roi(x, y, resizeWidth[k], resizeHeight);
                        cv::Size s = canvasImage(roi).size();
                        // change the number of channels to three
                        cv::Mat target_ROI(s, CV_8UC3);
                        if (vecMat[k].channels() != canvasImage.channels()) {
                            if (vecMat[k].channels() == 1) {
                                cv::cvtColor(vecMat[k], target_ROI, CV_GRAY2BGR);
                            }
                        } else {
                            vecMat[k].copyTo(target_ROI);
                        }
                        cv::resize(target_ROI, target_ROI, s);
                        if (target_ROI.type() != canvasImage.type()) {
                            target_ROI.convertTo(target_ROI, canvasImage.type());
                        }
                        target_ROI.copyTo(canvasImage(roi));
                        x_end += resizeWidth[k] + edgeThickness;
                }
        }
	return canvasImage;
}
