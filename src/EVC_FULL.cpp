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
#include "makeCanvas.cpp"
#include "arduinoCommand.cpp"

using namespace cv;
using namespace std;
// Global variables
enum DriveCommand {DRIVE_STRAIGHT=0,TURN_LEFT,TURN_RIGHT};
enum SignDetected {SIGN_NONE=0,SIGN_LEFT,SIGN_RIGHT, SIGN_STRAIGHT, SIGN_STOP, SIGN_U_TURN, SIGN_UNKNOWN};
cv::Mat src,dst,src_crop, dst_color, gray, thresh , hough;
cv::Mat src_path,dst_path,src_sign,dst_sign;
float pctCropHeight = 0.1; //0.1
int iThresh = 60;
int maxLines = 25;
int curved = 1, straight = 1;
int ITER;
int FrameSkip = 5;
const int compRatio = 2;
vector<Vec4i> lines;
double dWidth,dHeight;
char keyboard; //input from keyboard
bool video = true;

//Functions
void processVideo(char* videoFilename);
int findPath(InputArray src, OutputArray dst, bool display);
int findSign(InputArray src, OutputArray dst, bool display);
void processFrame(InputArray src,OutputArray dst,OutputArray hough, double dWidth, double dHeight);
void processFrame_adaptive(InputArray src,OutputArray dst,OutputArray hough, double dWidth, double dHeight);
int searchLongestLine_Angle(InputArray src,OutputArray dst, double dWidth, double dHeight);
int searchLongestLine_Straight(InputArray src,OutputArray dst, double dWidth, double dHeight);
void help();

void help()
{
    cout
    << "--------------------------------------------------------------------------" << endl
    << "This program is used for the Embedded Visual Control Project for Group 7."  << endl                                                                                   				     << endl
    << "Usage:"                                                                     << endl
    << "./EVC_FULL {-vid <video filename>}"                    			    << endl
    << "for example: ./EVC_FULL ./demo_1.h264"                                      << endl
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
	//int fd = serialOpen ("/dev/ttyACM0",9600);
	//if (fd<0){exit(EXIT_FAILURE);}
	//serialFlush(fd);
    //create the capture object
//	if (video){
    VideoCapture capture(videoFilename);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
	}
/*    } else {
	raspicam::RaspiCam_Cv capture;
	capture.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
	if (!capture.open()) {cerr<<"Error opening the capture"<<endl;exit(EXIT_FAILURE);}
*/	//}

    //Get screen sizes
    dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
	Size frameSize(static_cast<int>(dWidth), static_cast<int>(dHeight));
	//set filename
	time_t now = time(NULL);
	char ctime[256];
	strftime(ctime,256,"%d_%m_%Y_%H_%M", gmtime(&now));
	//cout << capture.get(CAP_PROP_FOURCC)<<"\n";
	//Create Videowriter instance
	VideoWriter oVideoWriter("video/" + string(ctime) + ".avi", CV_FOURCC('M','J','P','G'),5 ,frameSize , true);

    //Set Cropping parameters
    float yFl = float(dHeight) * pctCropHeight;
    int y = int(yFl + 0.5);
    cv::Rect myROI(0,y,dWidth,dHeight-y);

    //read input data. ESC or 'q' for quitting
    keyboard = 0;
    ITER = 0;
    while( keyboard != 'q' && keyboard != 27 ){
		double time_ = cv::getTickCount();
//		if (!video){
		    //read the current frame
/*		    if(! capture.grab()){
		        cerr << "Unable to read next frame." << endl;
		        cerr << "Exiting..." << endl;
		        exit(EXIT_FAILURE);
			
        	}
		capture.retrieve (src);
*//*		}else{
*/			if(!capture.read(src)) {
				    cerr << "Unable to read next frame." << endl;
				    cerr << "Exiting..." << endl;
				    exit(EXIT_FAILURE);
				}
//	}
        //Start processing frame
        if (ITER % FrameSkip==FrameSkip/5 && !src.empty() && ITER > 20){
			cout << "\n" << ITER << " processing image...";
			//Crop image to remove top part which is not of interest
			src_path = src(myROI);
			//oVideoWriter.write(src);
			int iDirPath = findPath(src_path,dst_path,false);
			int iDirSign = findSign(src_sign,dst_sign,false);

	
        //Give commands
			switch (iDirPath){
			case TURN_LEFT :{
				//ArduinoCommand(fd,30);
				//goLeft(20);
				break;
			}
			case TURN_RIGHT:{
				//ArduinoCommand(fd,60);
				//goRight(20);
				break;
			}
			case DRIVE_STRAIGHT:{
				//ArduinoCommand(fd,9);
				//goForward();
				break;
			}
			}
		double secondsElapsed = double (cv::getTickCount()-time_)/double(cv::getTickFrequency());
		cout << "Time :" << secondsElapsed << " FPS: " << 1/secondsElapsed;
        }
        //Finalize
        ITER++;
        keyboard = (char)waitKey( 30 );
    }
    cout<<"Stop capture..."<<endl;
    //delete capture object
    capture.release();
}

int findPath(InputArray src, OutputArray dst, bool display){
	vector<Mat> Images;
	processFrame(src,thresh,hough,dWidth,dHeight);
	//processFrame_adaptive(src_crop,thresh,hough,dWidth,dHeight);

	//Process results into a working roadmap
	int a_c = -1, a_s = -1;
	if (curved){a_c = searchLongestLine_Angle(hough,hough,dWidth/compRatio,dHeight/compRatio);}
	if (straight){a_s = searchLongestLine_Straight(hough,hough,dWidth/compRatio,dHeight/compRatio);}
	//Calculate direction
	double d_c, d_s;
	d_c = a_c - maxLines/2;
	d_s = a_s - maxLines/2;

	Mat command(dHeight,dWidth,CV_8UC3,Scalar(0,0,0));
	int direction = 0;
	if (curved && straight){
		if(	  abs(d_s)<maxLines/8){	direction = DRIVE_STRAIGHT;
		}else if( d_c>-maxLines/8){	direction = TURN_LEFT;
		}else if( d_c<maxLines/8){	direction = TURN_RIGHT;
		}
	}
	else if (curved){
		if(	  d_c>-maxLines/8){	direction = TURN_LEFT;
		}else if( d_c<maxLines/8){	direction = TURN_RIGHT;
		}else{				direction = DRIVE_STRAIGHT;
		}
	}
	else if (straight){
		if(	   d_s<-maxLines/8){	direction = TURN_LEFT;
		}else if(  d_s>maxLines/8){	direction = TURN_RIGHT;
		}else{	   			direction = DRIVE_STRAIGHT;
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
	Mat gray,canny, cmp(int(dHeight/compRatio),int(dWidth/compRatio),CV_8UC3);
        Mat canvas(dHeight,dWidth,CV_8UC3,Scalar(0,0,0));
	//Resize
	resize(src,cmp,cmp.size(),0,0,0);
	//Change color to Gray
	cvtColor(cmp,gray,CV_RGB2GRAY);
	//cout << "\n Set Threshold";
	threshold(gray,thresh,iThresh,255,THRESH_BINARY_INV);
	//Canny
//	Canny(thresh,canny,100,300,3);
	//cout << "\n Draw Houghlines";
	HoughLinesP( thresh, lines, 1, CV_PI/180, 200/compRatio, 250/compRatio, 0);
	for( size_t i = 0; i < lines.size(); i++ )
	{
		line(canvas, Point(lines[i][0], lines[i][1]),
			Point(lines[i][2], lines[i][3]), Scalar(255,255,255), 8, 3 );
	}
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





