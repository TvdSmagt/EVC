#include <stdio.h>
#include <iostream>
#include <wiringSerial.h>

using namespace std;
int main ()
{
	cout << "Starting...\n";
	int fd = serialOpen ("/dev/ttyACM0",9600);
	if (fd < 0){
	return -1;
	}
	serialPutchar (fd, 0);
	serialPutchar (fd, 0);
	for (int i = 0;i<127;i++){
	serialPutchar (fd, 0);
	serialPutchar (fd, 1);
	serialPutchar (fd, 2);
	serialPutchar (fd, 3);
	}
	serialFlush(fd);
	for (int i = 0;i<20;i++){
	//while(1){
	int avail = serialDataAvail(fd);
	cout << i << " Available:" << avail << "\n Sending Data ...";
	serialPutchar (fd, 4);
	serialPutchar (fd, 5);
	serialPutchar (fd, 6);
	serialPutchar (fd, 7);
	//while (serialDataAvail(fd) > 0){ //(1){
	for (int j = 0;j<4;j++){
	cout << "\nReceiving data: ";
	int answer = serialGetchar(fd);
	cout << answer << "\n";
	}}
	//while(1){
	/*serialPutchar (fd,1);
	serialGetchar(fd);
	serialPutchar (fd,1);
	serialPutchar (fd,1);
	serialPutchar (fd,1);
	serialPutchar (fd,1);*/
	//}
return 0;
}
