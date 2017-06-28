#include <stdio.h>
#include <iostream>
#include <wiringSerial.h>

#include "/home/pi/EVC/src/arduinoCommand.cpp"

using namespace std;


string command;

int main ()
{
	if(!ArduinoOpen()){
	    cout<<"Error connecting to the Arduino"; return -1;}

	while(1){
		cin >> command;
		if ((command == "s") || (command == "stop")){
			carStop();
		} else if((command == "l") || (command == "left")){
			goLeft(10,10);
		} else if((command == "r") || (command == "right")){
			goRight(10,10);
		} else if((command == "f") || (command == "forward")){
			goForward(10);
		}
	}
	/*cout << "Starting...\n";
	//int fd = serialOpen ("/dev/ttyACM0",9600);
	int fd = serialOpen ("/dev/ttyUSB0",9600);
	if (fd < 0){
	    return -1;
	}
	serialFlush(fd);
	while(1){
	    for (int a = 0;a<4;a++){
	        int ans;
	        cin >> ans;
	    	cout << "Sending: " << ans << "\n";
	        serialPutchar (fd,ans);
	    }
	    for (int j = 0;j<4;j++){
	        cout << "\nReceiving data: ";
	        int answer = serialGetchar(fd);
	        cout << answer << "\n";
	    }
	}*/
return 0;
}
