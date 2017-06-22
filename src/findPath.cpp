using namespace cv;
using namespace std;

//Variables
enum DriveCommand {DRIVE_STRAIGHT=0,TURN_LEFT,TURN_RIGHT};
float pctCropTop = 0.60; //0.1 - Previous: 0.1
float pctCropBottom = 0.25;
int iThresh = 80; //outside: 60
const int maxLines = 100;
const int curved = 1;
const int straight = 1;
const int compRatio = 2;
const int SHOW_LINES = 1;
vector<Vec4i> lines;

//Functions
int findPath(InputArray src, OutputArray dst, bool display, double dWidth, double dHeight);
void processFrame(InputArray src,OutputArray dst,OutputArray hough, double dWidth, double dHeight);
void processFrame_adaptive(InputArray src,OutputArray dst,OutputArray hough, double dWidth, double dHeight);
int searchLongestLine_Angle(InputArray src,OutputArray dst, double dWidth, double dHeight);
int searchLongestLine_Straight(InputArray src,OutputArray dst, double dWidth, double dHeight);
int findFreeSpace(InputArray src, OutputArray dst, double dWidth, double dHeight);

int findPath(InputArray src, OutputArray dst, bool display, double dWidth, double dHeight){
	Mat thresh,hough;
	vector<Mat> Images;
	Mat command(dHeight,dWidth,CV_8UC3,Scalar(0,0,0));processFrame(src,thresh,hough,dWidth,dHeight);
	//processFrame_adaptive(src_crop,thresh,hough,dWidth,dHeight);
/*
	//Process results into a working roadmap
	int a_c = -1, a_s = -1;
	if (curved){a_c = searchLongestLine_Angle(hough,hough,dWidth/compRatio,dHeight/compRatio);}
	if (straight){a_s = searchLongestLine_Straight(hough,hough,dWidth/compRatio,dHeight/compRatio);}
	//Calculate direction
	double d_c, d_s;
	d_c = a_c - maxLines/2;
	d_s = a_s - maxLines/2;

	int direction = 0;
	if (curved && straight){
		if(	  abs(d_s-maxLines/8)<maxLines/8){	direction = DRIVE_STRAIGHT;cout<<"\tGo Forward";
		}else if( d_c>-maxLines/8){	direction = TURN_LEFT;cout<<"\tTurn Left";
		}else if( d_c<maxLines/8){	direction = TURN_RIGHT;cout<<"\tTurn Right";
		}
	}
	else if (curved){
		if(	  d_c>-maxLines/8){	direction = TURN_LEFT;cout<<"Turn Left\n";
		}else if( d_c<maxLines/8){	direction = TURN_RIGHT;cout<<"Turn Right\n";
		}else{				direction = DRIVE_STRAIGHT;cout<<"Go Forward\n";
		}
	}
	else if (straight){
		if(	   d_s<-maxLines/8){	direction = TURN_LEFT;cout<<"Turn Left\n";
		}else if(  d_s>maxLines/8){	direction = TURN_RIGHT;cout<<"Turn Right\n";
		}else{	   			direction = DRIVE_STRAIGHT;cout<<"Go Forward\n";
		}
	}
*/
	int direction = findFreeSpace(hough,hough,dWidth/compRatio,dHeight/compRatio);
	hough.copyTo(dst);
	if (display){
	//Display different versions
	Mat srcc;
	src.copyTo(srcc);
	Images.push_back(srcc);
	Images.push_back(thresh);
	Images.push_back(hough);
	Images.push_back(command);
	Mat canvas = makeCanvas(Images, dHeight,3);
	namedWindow("Canvas", WINDOW_NORMAL);
	cv::resizeWindow("Canvas", dWidth/2, dHeight/2);
	imshow("Canvas",canvas);
	}
	return direction;
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
	//Canny(thresh,canny,20,50,3);
	cout << "\n Draw Houghlines";
	HoughLinesP( thresh, lines, 1, CV_PI/180, 0/compRatio, 0/compRatio, 0);//200,100,0
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
				if(SHOW_LINES){line(temp, cen, p2, Scalar(0,255,0), 2, 3 );}
				max_angle.push_back(i);
				max_range.push_back(r);
				break;
			}
		}
	}
	int x_max, r = 0;
		for (uint64 i = 0;i<max_range.size();i++){
			if ((r < max_range[i]) || ((r <= max_range[i])&&(abs(max_angle[i]-maxLines/2-maxLines/8) < abs(x_max-maxLines/2-maxLines/8)))){
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
				if(SHOW_LINES){line(temp, cen, p2, Scalar(0,255,0), 2, 3 );}
				max_angle.push_back(i);
				max_range.push_back(r);
				break;
			}
		}
	}
	int angle, r = 0;
	for (uint64 i = 0;i<max_range.size();i++){
		if ((r < max_range[i]) || ((r <= max_range[i])&&(abs(max_angle[i]+maxLines/2) < abs(angle+maxLines/2)))){
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
int findFreeSpace(InputArray src, OutputArray dst, double dWidth, double dHeight){
	vector<int> max_angle,max_range;
	int direction = DRIVE_STRAIGHT;
	int favor_left = 0, favor_right = 0;
	bool nospace = false;
	Mat temp;
	src.copyTo(temp);
	Rect bounds(0, 0, dWidth, dHeight);
	for(int i = 2; i<=maxLines-2; i++) // Loop over lines
	{
		Point cen(int(dWidth*i/maxLines),int(dHeight));
		for (int r = 5; r <= dHeight+1; r++) //Find Line End
		{
			Point p2(dWidth*i/maxLines,dHeight-r);
			if (!bounds.contains(p2) || (temp.at<Vec3b>(p2).val[2] >0)){
				if(SHOW_LINES){line(temp, cen, p2, Scalar(0,255,0), 2, 3 );}
				max_angle.push_back(i);
				max_range.push_back(r);
				break;
			}
		}
	}
	int dHeight_New = dHeight * 0.5 / compRatio;
	int dWidth_Start = int(dWidth/7);
	int dWidth_Stop = int(dWidth*6/7);
	Point d1l(dWidth_Start,dHeight);
	Point d1h(dWidth_Start,dHeight_New);
	Point d2l(dWidth_Stop,dHeight);
	Point d2h(dWidth_Stop,dHeight_new);

	if(SHOW_LINES){
		line(temp,(dWidth_Start,dHeight),(dWidth_Start,dHeight_New),Scalar(255,0,0),2,3);
		line(temp,(dWidth_Stop,dHeight),(dWidth_Stop,dHeight_New),Scalar(255,0,0),2,3);
		line(temp,(dWidth_Start,dHeight),(dWidth_Stop,dHeight_New),Scalar(255,0,0),2,3);
	}
	for(int j = maxLines/7; j <= maxLines * 6/7; j++){
		if ((direction == DRIVE_STRAIGHT)&&(max_range[j] < dHeight_New)){
			nospace = true;
			if (j < maxLines / 2){
				favor_right++;
			} else {
				favor_left++;
			}
		}
	}
	if (nospace){
		if (favor_left > favor_right){
			direction = TURN_LEFT; cout << "\tTurn Left";
		} else {
			direction = TURN_RIGHT;cout << "\tTurn Right";
		}
	} else cout << "\tGo Straight " << favor_right << " " << favor_left;
	temp.copyTo(dst);
	return direction;

}
