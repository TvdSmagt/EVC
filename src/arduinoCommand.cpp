
using namespace cv;
using namespace std;

//Variables
int ardAnswer[4];
const int NEUTRAL = 127;

//Functions
void ArduinoCommand(int fd, int command);
void ArduinoCommand(int fd, int Power, int Steer);
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
void ArduinoCommand2(int fd, int Power, int degrees){
	int Steer = 127;
	if ((degrees > 127)||(degrees < -127)) {
		Steer = 127 - 127 * Sign(degrees);
	} else {
		Steer -= degrees;
	}
	if(fd>0){
	serialPutchar(fd,Power);
	serialPutchar(fd,Steer);
	serialPutchar(fd,Power);
	serialPutchar(fd,Steer);
	}
	for (int r = 0; r<4;r++){
	ardAnswer[r] = serialGetchar(fd);
	}		
}
void goLeft(int fd, int degrees, int power){
	cout << "Turn Left\n";
	//ArduinoCommand2(fd,-degrees,power);
}
void goRight(int fd, int degrees, int power){
	cout << "Turn Right\n";
	//ArduinoCommand2(fd,degrees,power);
}
void goForward(int fd, int power){
	cout << "Go Forward\n";
	//ArduinoCommand2(fd,0,power);
}
void goBackward(int fd, int power){
	cout << "Go Backward\n";
	//ArduinoCommand2(fd,0,-power);
}
void Stop(int fd){
	cout << "Stop!\n";
	//ArduinoCommand2(fd,0,0);
}