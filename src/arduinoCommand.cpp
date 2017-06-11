
using namespace cv;
using namespace std;

//Variables
int ardAnswer[4];
const int NEUTRAL = 100;

//Functions
void ArduinoCommand(int fd, int command);
void ArduinoCommand2(int fd, int power, int degrees);
void goLeft(int fd, int degrees, int power);
void goRight(int fd, int degrees, int power);
void goForward(int fd, int power);
void goBackward(int fd, int power);
void Stop(int fd);

void ArduinoCommand(int fd, int command){
	for(int i = 0;i<4;i++){
	serialPutchar(fd,command);
	}
	for(int j = 0;j<4;j++){
	int answer = serialGetchar(fd);
	cout << answer << "\n";
	}
}
void ArduinoCommand2(int fd, int power, int degrees){
	int Steer = NEUTRAL;
	int Force = NEUTRAL;
	if (degrees > NEUTRAL){		Steer = 2*NEUTRAL;}
	 else if(degrees <- NEUTRAL){	Steer = 0;}
 	 else {				Steer += degrees;}
	if (power > NEUTRAL){		Force = 2*NEUTRAL;}
	 else if(power <- NEUTRAL){	Force = 0;} 
	 else {				Force += power;}
	cout << "Sending Message: " << Force << " " << Steer << " " << Force << " " << Steer << "\n";
//Send Force and Steer values
	serialPutchar(fd,int(Force));
	serialPutchar(fd,int(Steer));
//Control bytes -> Can maybe be used for other data later...
	serialPutchar(fd,int(Force));
	serialPutchar(fd,int(Steer));
//	cout << "Answer:\n";
/*	for (int r = 0; r<4;r++){
	int answer = serialGetchar(fd);
	cout << answer << "\n";
//	ardAnswer[r] = serialGetchar(fd);
//	cout << ardAnswer[r];
	}		
*///	cout << "\nEnd Transmission\n";
}
void goLeft(int fd, int degrees, int power){
	cout << "Turn Left\n";
	ArduinoCommand2(fd,-degrees,power);
}
void goRight(int fd, int degrees, int power){
	cout << "Turn Right\n";
	ArduinoCommand2(fd,degrees,power);
}
void goForward(int fd, int power){
	cout << "Go Forward\n";
	ArduinoCommand2(fd,0,power);
}
void goBackward(int fd, int power){
	cout << "Go Backward\n";
	ArduinoCommand2(fd,0,-power);
}
void Stop(int fd){
	cout << "Stop!\n";
	ArduinoCommand2(fd,0,0);
}
