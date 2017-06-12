//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/objdetect.hpp>
#include <raspicam/raspicam_cv.h>
//C
#include <stdio.h>
//C++
#include <iostream>
#include <vector>
#include <sstream>
//local
#include "makeCanvas.cpp"
#include "arduinoCommand.cpp"
#include "findPath.cpp"
#include "findSign.cpp"

using namespace cv;
using namespace std;

// Global variables
int ITER = 0;
int FrameSkip = 5;
double dWidth,dHeight;
char keyboard = 0 ; //input from keyboard
const int INPUT_VIDEO = 1; //1=VIDEO, 0=CAMERA
const int SAVE_VIDEO = 1; //1=SAVE,0=DONT SAVE
const int DISPLAY_VIDEO = 0; //1=DISPLAY,0=NO_DISPLAY
const int ARDUINO_CONNECT = 1; //1=CONNECT, 0=DONT_CONNECT
VideoWriter oVideoWriter;

//Functions
void help();
void processVideo(char* videoFilename);


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
    if(ARDUINO_CONNECT){
	if(!ArduinoOpen()){
	    exit(EXIT_FAILURE);}}

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
	if(SAVE_VIDEO){oVideoWriter = VideoWriter("video/" + string(ctime) + ".avi", CV_FOURCC('M','J','P','G'),5 ,frameSize , true);}

    //Set Cropping parameters
    float yFl = float(dHeight) * pctCropHeight;
    int y = int(yFl + 0.5);
    cv::Rect myROI(0,y,dWidth,dHeight-y);
    Mat src, src_path, src_sign, dst_path, dst_sign;

    //read input data. ESC or 'q' for quitting
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
			if(SAVE_VIDEO){oVideoWriter.write(src);}
			int iDirPath = findPath(src_path,dst_path,DISPLAY_VIDEO, dWidth, dHeight);
			int iDirSign = findSign(src_sign,dst_sign,DISPLAY_VIDEO, dWidth, dHeight);

        //Give commands
			switch (iDirPath){
			case TURN_LEFT :{
				if(ARDUINO_CONNECT){goLeft(5,10);}
				break;
			}
			case TURN_RIGHT:{
				if(ARDUINO_CONNECT){goRight(5,10);}
				break;
			}
			case DRIVE_STRAIGHT:{
				if(ARDUINO_CONNECT){goForward(10);}
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
