#include <wiringSerial.h>


//using namespace cv;
using namespace std;

//Variables
const int NEUTRAL = 100;
int fd;

//Functions
bool ArduinoOpen();
void ArduinoCommand(int command);
void ArduinoCommand2(int power, int degrees);
void goLeft(int degrees, int power);
void goRight(int degrees, int power);
void goForward(int power);
void goBackward(int power);
void carStop();
void uTurn();

bool ArduinoOpen(){
	cout<<"\nOpening USB connection w/Arduino\nTrying USB0...";
	fd = serialOpen ("/dev/ttyUSB0",9600);
	if (fd<0){cout<<"Failed!\n Trying USB1...";fd = serialOpen ("/dev/ttyUSB1",9600);}
	if (fd<0){cout<<"Failed!\n Trying ACM0...";fd = serialOpen ("/dev/ttyACM0",9600);}
	if (fd<0){cerr <<"Failed!\nUnable to open connection to ARDUINO\n"; return false;}
	serialFlush(fd);
	return 1;
}

void ArduinoCommand(int command){
	for(int i = 0;i<4;i++){
	serialPutchar(fd,command);
	}
	for(int j = 0;j<4;j++){
	int answer = serialGetchar(fd);
	cout << answer << "\n";
	}
}
void ArduinoCommand2(int power, int degrees){
	int Steer = NEUTRAL;
	int Force = NEUTRAL;
	if (degrees > NEUTRAL){		Steer = 2*NEUTRAL;}
	 else if(degrees <- NEUTRAL){	Steer = 0;}
 	 else {				Steer += degrees;}
	if (power > NEUTRAL){		Force = 2*NEUTRAL;}
	 else if(power <- NEUTRAL){	Force = 0;} 
	 else {				Force += power;}
//	cout << "Sending Message: " << Force << " " << Steer << " " << Force << " " << Steer << "\n";
//Send Force and Steer values
	serialPutchar(fd,int(Force));
	serialPutchar(fd,int(Steer));
//Control bytes -> Can maybe be used for other data later...
	serialPutchar(fd,int(Force));
	serialPutchar(fd,int(Steer));
}
void goLeft(int degrees, int power){
//	cout << "Turn Left\n";
	ArduinoCommand2(power,degrees);
}
void goRight(int degrees, int power){
//	cout << "Turn Right\n";
	ArduinoCommand2(power,-degrees);
}
void goForward(int power){
//	cout << "Go Forward\n";
	ArduinoCommand2(power,0);
}
void goBackward(int power){
//	cout << "Go Backward\n";
	ArduinoCommand2(-power,0);
}
void carStop(){
//	cout << "Stop!\n";
	ArduinoCommand2(0,0);
}
void uTurn(){
	goBackward(20);
	goLeft(90,10);
	goLeft(90,10);
}
