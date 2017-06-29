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
double fps_total = 0;
int FrameSkip = 1;
double dWidth,dHeight;
char keyboard = 0 ; //input from keyboard
const int INPUT_VIDEO = 0; //1=VIDEO, 0=CAMERA
const int SAVE_VIDEO = 1; //1=SAVE, 0=DONT SAVE
const int DISPLAY_VIDEO = 1; //1=DISPLAY,0=NO_DISPLAY
const int ARDUINO_CONNECT = 1; //1=CONNECT, 0=DONT_CONNECT
const int P_DRIVE = 4;
int angle_turned = 0;
int SignDirection = SIGN_NONE;
int prefDirection;
Mat src, src_path, src_sign, dst_path, dst_sign;
Size frameSize;//(static_cast<int>(dWidth), static_cast<int>(dHeight));

//Video Capture/Writing
VideoWriter oVideoWriter;
VideoCapture vCapture;
raspicam::RaspiCam_Cv cCapture;

//Cropping
int y_top,y_bot,y_Sum;
cv::Rect myROI;

//Functions
void help();
void processVideo(char* videoFilename);
void VideoCapture_Open(char* videoFilename);
void VideoCapture_Grab();
void VideoWriter_Open();
void Cropping_Calc();
void SendDrivingCommands();
void FPS_Calc(double time_);
void KeyboardSign(char keyboard);

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
    if(ARDUINO_CONNECT){if(!ArduinoOpen()){exit(EXIT_FAILURE);}}   //Open Serial Connection With Arduino
	VideoCapture_Open(videoFilename);		//Open Source Video
	if(SAVE_VIDEO){VideoWriter_Open();}		//Open Save Video
    Cropping_Calc();						//Calculate the cropping area;
	cout <<"\n--------------------------------------------------------------------------";
	cout <<"\nStart reading input from source.";
	cout <<"\n--------------------------------------------------------------------------";

//read input data. ESC or 'q' for quitting
// START LOOP
    while( keyboard != 'q' && keyboard != 27 ){
		double time_ = cv::getTickCount(); 	//For FPS Calc
		VideoCapture_Grab(); 				//Capture Frame
		KeyboardSign(keyboard); 			//Placeholder Sign Detection
       //Start processing frame
        if (ITER % FrameSkip==0 && !src.empty() && ITER > 0){
			cout << "\n" << ITER << "\t Processing image... : ";
			//Crop image to remove top part which is not of interest
			src_path = src(myROI);
			if(SAVE_VIDEO){oVideoWriter.write(src);}

			//Find Sign and Direction
			//int iDirSign = findSign(src_sign,dst_sign,DISPLAY_VIDEO, dWidth, dHeight - y_Sum);
			int iDirSign = SignDirection;
			if (iDirSign == SIGN_U_TURN){
				uTurn();
			} else {
				SendDrivingCommands();
			}
			FPS_Calc(time_); 
        }
        //Finalize
        ITER++;
        keyboard = (char)waitKey( 30 );
    }
	if(ARDUINO_CONNECT){carStop();}
    cout<<"\nStop capture..."<<endl;cout<< "\tAverage fps: " << fps_total/ITER << "\n";
    //delete capture object
    if(INPUT_VIDEO){
	vCapture.release();
    }else{
	cCapture.release();
    }
}

void VideoCapture_Open(char* videoFilename){
	cout <<"\n--------------------------------------------------------------------------";
	if (INPUT_VIDEO){
		cout << "\nTrying to open Video From file located in: \n" << videoFilename << "\t";
		vCapture = VideoCapture(videoFilename);
		if(!vCapture.isOpened()){cerr <<"Unable to open the video file.\n"<<endl; exit(EXIT_FAILURE);}
		cout << "Success!";
	 	//Get screen sizes
    		dWidth = vCapture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
    		dHeight = vCapture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video
	}else{
		cout << "\nTrying to open Video from Raspberry Camera: \n";
		cCapture.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
		if (!cCapture.open()) {cerr<<"Unable to open the camera feed.\n"<<endl;exit(EXIT_FAILURE);}
		 //Get screen sizes
		cout << "Success!";
		dWidth = cCapture.get(CV_CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
		dHeight = cCapture.get(CV_CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video			
	}
	frameSize = Size(static_cast<int>(dWidth), static_cast<int>(dHeight));

	cout <<"\n--------------------------------------------------------------------------";
}

void VideoCapture_Grab(){
	if (!INPUT_VIDEO){
			if(!cCapture.grab()){cerr<<"\nUnable to read camera frame.\n"<<endl;exit(EXIT_FAILURE);}
			cout << "\nCapturing Camera";
			cCapture.retrieve(src);
		}else{
			if(!vCapture.read(src)){cerr<<"\nUnable to read video frame.\n"<<endl;cout<< "\tAverage fps: " << fps_total/ITER << "\n";exit(EXIT_FAILURE);}
		}

}

void VideoWriter_Open(){
	cout <<"\n--------------------------------------------------------------------------";
	cout << "\nOpening Filewriter.. ";
	//set filename
	time_t now = time(NULL);
	char ctime[256];
	strftime(ctime,256,"%d_%m_%Y_%H_%M", gmtime(&now));
	//Create Videowriter instance
	oVideoWriter = VideoWriter("videos/" + string(ctime) + ".avi", CV_FOURCC('M','J','P','G'),5 ,frameSize , true);
	cout<<"\nSaving recorded video in: \n" << "./videos/" << string(ctime) + ".avi";
	cout <<"\n--------------------------------------------------------------------------";
}

void Cropping_Calc(){
	//Set Cropping parameters
    float yFl_top = float(dHeight) * pctCropTop;
	float yFl_bot = float(dHeight) * pctCropBottom;
    y_top = int(yFl_top + 0.5);//0.5 for rounding up
    y_bot = int(yFl_bot + 0.5);
	y_Sum = y_top + y_bot;
	myROI = cv::Rect(0,y_top,dWidth,dHeight-y_Sum);
}

void SendDrivingCommands(){
	int iDirPath = findPath(src_path,dst_path,DISPLAY_VIDEO, dWidth, dHeight - y_Sum, prefDirection);	
	switch (iDirPath){
		case TURN_LEFT :{
			if(prefDirection==PREF_LEFT){
				angles += P_DRIVE;
				if(angles > 80){prefDirection = SIGN_NONE;angles=0;cout<<"Stop Searching";}
			}
			if(ARDUINO_CONNECT){goLeft(P_DRIVE,10);}
			break;
		}
		case TURN_RIGHT:{
			if(prefDirection == PREF_RIGHT){
				angles += P_DRIVE;
				if(angles > 80){prefDirection = SIGN_NONE;angles=0;cout<<"Stop Searching";}
			}
			if(ARDUINO_CONNECT){goRight(P_DRIVE,10);angles+=P_DRIVE;}
			break;
		}
		case DRIVE_STRAIGHT:{
			if(ARDUINO_CONNECT){goForward(P_DRIVE);}
			break;
		}
	}

}

void FPS_Calc(double time_){
		double secondsElapsed = double (cv::getTickCount()-time_)/double(cv::getTickFrequency());
		double fps = 1 / secondsElapsed;
		fps_total += fps;
		cout << "\tTime :" << secondsElapsed << "\tFPS: " << fps;
}

void KeyboardSign(char keyboard){
	if (keyboard == 'l'){SignDirection = PREF_LEFT;prefDirection = PREF_LEFT;}
	else if (keyboard == 'r'){SignDirection = PREF_RIGHT;prefDirection = PREF_RIGHT;}
	else if (keyboard == 'u'){SignDirection = SIGN_U_TURN;}
	else if (keyboard == 's'){SignDirection = SIGN_STOP;}
	else {SignDirection = SIGN_NONE;}
}
