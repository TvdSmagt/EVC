#include <wiringSerial.h>


//using namespace cv;
using namespace std;

//Variables
const int NEUTRAL = 100;
int fd;

//Functions
bool ArduinoOpen();
void ArduinoCommand(int power, int degrees);
void SendCommand(int Force, int Steer);
void ReceiveCommand();
void goLeft(int degrees, int power);
void goRight(int degrees, int power);
void goForward(int power);
void goBackward(int power);
void carStop();
void uTurn();
int RcvBits[4];

bool ArduinoOpen(){
	cout<<"\n----------------------------------------------------";
	cout<<"\nOpening USB connection w/Arduino\nTrying USB0...";
	fd = serialOpen ("/dev/ttyUSB0",9600);
	if (fd<0){cout<<"\tFailed!\nTrying USB1...";fd = serialOpen ("/dev/ttyUSB1",9600);}
	if (fd<0){cout<<"\tFailed!\nTrying ACM0...";fd = serialOpen ("/dev/ttyACM0",9600);}
	if (fd<0){cerr <<"\tFailed!\nUnable to open connection to ARDUINO\n"; return false;} else {cout << "\tSuccess!";}
	serialFlush(fd);
	cout<<"\n----------------------------------------------------";
	return 1;
}

void ArduinoCommand(int power, int degrees){
	int Steer = NEUTRAL;
	int Force = NEUTRAL;
	if (degrees > NEUTRAL){		Steer = 2*NEUTRAL;}
	 else if(degrees <- NEUTRAL){	Steer = 0;}
 	 else {				Steer += degrees;}
	if (power > NEUTRAL){		Force = 2*NEUTRAL;}
	 else if(power <- NEUTRAL){	Force = 0;} 
	 else {				Force += power;}
//	cout << "Sending Message: " << Force << " " << Steer << " " << Force << " " << Steer << "\n";
	SendCommand(Force,Steer);
	ReceiveCommand();
}

void SendCommand(int Force, int Steer){
//	serialFlush(fd);
// Control Bytes
	serialPutchar(fd,222);
	serialPutchar(fd,222);
//Send Force and Steer values
	serialPutchar(fd,int(Force));
	cout << Force << " " << Steer;
	serialPutchar(fd,int(Steer));
	//Control bytes -> Can maybe be used for other data later...
//	serialPutchar(fd,int(Force));
//	serialPutchar(fd,int(Steer));
}

void ReceiveCommand(){
	if (serialDataAvail(fd)>4){
	while (RcvBits[0]!= 97){
		RcvBits[0] = serialGetchar(fd);
		cout << "\t" << RcvBits[0];
	}
		for(int i = 1;i<4;i++){
			RcvBits[i] = serialGetchar(fd);
			cout << "\t" << RcvBits[i];
		}
	}
}
void goLeft(int degrees, int power){
	ArduinoCommand(power,degrees);
}
void goRight(int degrees, int power){
	ArduinoCommand(power,-degrees);
}
void goForward(int power){
	ArduinoCommand(power,0);
}
void goBackward(int power){
	ArduinoCommand(-power,0);
}
void carStop(){
//	cout << "Stop!\n";
	ArduinoCommand(0,0);
}
void uTurn(){
	cout<<"\nU TURN!";
/*	goBackward(5);
	goLeft(90,10);
	goLeft(90,10);
*/
	SendCommand(255,255);
}
