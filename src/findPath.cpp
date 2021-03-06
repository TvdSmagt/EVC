//#include "findSign.cpp"

using namespace cv;
using namespace std;

//Variables
enum DriveCommand {DRIVE_STRAIGHT=0,TURN_LEFT,TURN_RIGHT};
enum SignCommand {PREF_NONE = 0, PREF_LEFT,PREF_RIGHT,PREF_STRAIGHT};
float pctCropTop = 0.25; //0.1 - Previous: 0.1
float pctCropBottom = 0.45;
int iThresh = 65; //outside: 60
int aThresh = 20;
double mThresh = 0.8;
const int maxLines = 220;
const int curved = 1;
const int straight = 1;
const int compRatio = 2;
const int SHOW_LINES = 1;
const int SAVE_STEPS = 0;
const String img_loc = "/stills/";
vector<Vec4i> lines;
float pctFree = 0.10;
int md = 6;
int mt = 16;
int dirThresh = 3; //10;
int borThresh = 1;
int angles = 0;
bool leftFree, rightFree;
vector<int> max_angle,max_range;

//Functions
int findPath(InputArray src, OutputArray dst, bool display, double dWidth, double dHeight, int iDirSign);
void processFrame(InputArray src,OutputArray dst,OutputArray hough, double dWidth, double dHeight);
void processFrame_adaptive(InputArray src,OutputArray dst,OutputArray hough, double dWidth, double dHeight);
int findFreeSpace(InputArray src, OutputArray dst, double dWidth, double dHeight);

int findPath(InputArray src, OutputArray dst, bool display, double dWidth, double dHeight, int iDirSign){
	Mat thresh,hough;
	vector<Mat> Images;
	Mat command(dHeight,dWidth,CV_8UC3,Scalar(0,0,0));processFrame(src,thresh,hough,dWidth,dHeight);
	
	int direction = findFreeSpace(hough,hough,dWidth/compRatio,dHeight/compRatio);
	if ((iDirSign == PREF_LEFT)&&(direction == DRIVE_STRAIGHT)){
		cout << "Search Left!";
		leftFree = true;		
		for (int i = 2; i <= maxLines / 12; i++){
			//cout << "\n" << (max_range[i]/dHeight) << " < " << double(0.5);
			if ((max_range[i]/dHeight) < 0.5){
				leftFree = false;
			}
		}
		if (leftFree==true){
			direction = TURN_LEFT;
			cout <<"Turn Left (Prefer)";
		}
	} else if ((iDirSign == PREF_RIGHT)&&(direction == DRIVE_STRAIGHT)){
		cout <<"Search Right!";
		rightFree = true;		
		for (int i = maxLines * 11 /12; i <= maxLines; i++){
			if (max_range[i]/dHeight < (1-pctFree)){
				rightFree = false;
			}
		}
		if (rightFree){
			direction = TURN_RIGHT;
			cout <<"Turn Right (Prefer)";
		}
	}
	max_range.clear();
	max_angle.clear();
	if(SAVE_STEPS){imwrite("./stills/findPath.png",hough);}
	hough.copyTo(dst);
	if (display){
	//Display different versions
	Mat srcc;
	src.copyTo(srcc);
	Images.push_back(srcc);
	Images.push_back(thresh);
	Images.push_back(hough);
//	Images.push_back(command);
	Mat canvas = makeCanvas(Images, dHeight*2*compRatio,3);
	namedWindow("Canvas", WINDOW_NORMAL);
	cv::resizeWindow("Canvas", dWidth, dHeight);
	imshow("Canvas",canvas);
	}
	return direction;
}

void processFrame(InputArray src,OutputArray thresh,OutputArray hough, double dWidth, double dHeight) {
	//Initialize Matrices 
	Mat gray,canny, dil,kernel, cmp(int(dHeight/compRatio),int(dWidth/compRatio),CV_8UC3),canvas(dHeight/compRatio,dWidth/compRatio,CV_8UC3,Scalar(0,0,0));
	resize(src,cmp,cmp.size(),0,0,0); 											//Resize to reduce points and improve performance
	cvtColor(cmp,gray,CV_RGB2GRAY); 											//Convert to grayscale for improved performance
	cv::Scalar iThresh_avg = mean(src);											//Calculate average value of the image
	cout << iThresh_avg[0];
	threshold(gray,thresh,int(iThresh_avg[0])*mThresh,255,THRESH_BINARY_INV); 			//Set Threshold
//	adaptiveThreshold(gray,thresh,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY_INV,3,0);
	Canny(thresh,canny,0,0);
	int kernel_size = 3;
	kernel = Mat::ones(kernel_size,kernel_size,CV_32F)/ (float)(kernel_size*kernel_size);
	dilate(canny,dil,kernel);

	HoughLinesP( dil, lines, 1, CV_PI/180, 40/compRatio, 0/compRatio, 0);	//Calculate the HoughLines
	for( size_t i = 0; i < lines.size(); i++ )									//Draw Houghlines
	{
		line(canvas, Point(lines[i][0], lines[i][1]),
			Point(lines[i][2], lines[i][3]), Scalar(255,255,255), 2, 3 );
	}
	if(SAVE_STEPS){
	imwrite("./stills/cropped.png",src);
	imwrite("./stills/compressed.png",cmp);
	imwrite("./stills/gray.png",gray);
	imwrite("./stills/thresh.png",thresh);
	imwrite("./stills/canny.png",canny);
	imwrite("./stills/dilate.png",dil);
	imwrite("./stills/hough.png",canvas);
	}
//	dil.copyTo(thresh);
//	dil.copyTo(hough);
	canvas.copyTo(hough);														//Print Canvas
}
int findFreeSpace(InputArray src, OutputArray dst, double dWidth, double dHeight){
	int direction = DRIVE_STRAIGHT;
	long double favor_left = 0, favor_right = 0;
	bool nospace = false;
	Mat temp;
	src.copyTo(temp);
	Rect bounds(0, 0, dWidth, dHeight);
	for(int i = 1; i<=maxLines-1; i++) // Loop over lines
	{
		Point cen;
//		if((i > (maxLines /4)) && (i < (maxLines - (maxLines/4)))){
//			cen = Point(int(dWidth*i/maxLines),int(dHeight*0.8));
//		} else {
			cen = Point(int(dWidth*i/maxLines),int(dHeight));
//		}
		for (int r = 5; r <= dHeight+1; r++) //Find Line End
		{
			Point p2(dWidth*i/maxLines,dHeight-r);
			if (!bounds.contains(p2) || (temp.at<Vec3b>(p2).val[2] >0)){
				if(SHOW_LINES){line(temp, cen, p2, Scalar(0,255,0), 2, 3 );}
				max_angle.push_back(dWidth*i/maxLines);
				max_range.push_back(r);
				break;
			}
		}
	}
	int dHeight_New = dHeight * pctFree;// / compRatio;
	int dWidth_Start = int(dWidth/md);
	int dWidth_Stop = int(dWidth*(md-1)/md);
	Point d1l(dWidth_Start,dHeight);
	Point d1h(dWidth_Start,dHeight_New);
	Point d2l(dWidth_Stop,dHeight);
	Point d2h(dWidth_Stop,dHeight_New);
	int dWidth_Left = int(dWidth/mt);
	int dWidth_Right = int(dWidth*(mt-1)/mt);
	Point l1h(dWidth_Left,0);
	Point l1l(dWidth_Left,dHeight);
	Point r1h(dWidth_Right,0);
	Point r1l(dWidth_Right,dHeight);

	if(SHOW_LINES){
		line(temp,d1l,d1h,Scalar(255,0,0),2,3);
		line(temp,d1h,d2h,Scalar(255,0,0),2,3);
		line(temp,d2l,d2h,Scalar(255,0,0),2,3);
		line(temp,l1h,l1l,Scalar(0,0,255),2,3);
		line(temp,r1h,r1l,Scalar(0,0,255),2,3);
	}
	for(int j = 1; j <= maxLines-1; j++){ //int j = int((maxLines)/md); j <= int((maxLines) * (md-1)/md); j++
		if ((max_angle[j] >= dWidth_Start) && (max_angle[j] <= dWidth_Stop) && (max_range[j] < dHeight * (1-pctFree))){ //(direction == DRIVE_STRAIGHT)
			nospace = true;
			if (j < maxLines / 2){
				favor_right+= 1 - (max_range[j]/dHeight);
			} else {
				favor_left+= 1 - (max_range[j]/dHeight);
			}
		}
	}
	if (nospace){
	if ((favor_left > borThresh)&&(favor_right > borThresh)){
		favor_right = 0;
		favor_left = 0;
		//cout << "\t" << (maxLines / 8) << " \t" << ((maxLines * 7)/8);
		for(int j = 3; j <= maxLines-2; j++){ //int j = int((maxLines)/md); j <= int((maxLines) * (md-1)/md); j++
				if (j < (maxLines / mt)){
					favor_right+= 1 - (max_range[j]/dHeight);
					//cout << "\tl: " << j;
					//cout << "\nl" << (1-(max_range[j]/dHeight));
				}
				if (j > (maxLines - (maxLines/mt))){
					favor_left+= 1 - (max_range[j]/dHeight);
					//cout << "\tr: " << j;
					//cout << "\nr" << (1-(max_range[j]/dHeight));
				}
				//cout << "\t " << (max_range[j]/dHeight);
		}
		}
	if ((favor_left > dirThresh)||(favor_right > dirThresh)){
		if (favor_left > favor_right){
			direction = TURN_LEFT; 
			cout << "\tTurn Left   " << favor_left << " " << favor_left ;
		} else if(favor_right > favor_left){
			direction = TURN_RIGHT;
			cout << "\tTurn Right  " << favor_left << " " << favor_right;
		} else {cout << "\tGo Straight " << favor_left << " " << favor_right << " ";}
	} else {cout << "\tGo Straight " << favor_left << " " << favor_right << " ";
	}} else cout << "\tGo Straight " << favor_left << " " << favor_right << " ";
	
	temp.copyTo(dst);
//	cout << "Direction" << direction;
	return direction;

}
