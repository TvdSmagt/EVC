#include "mySerial.h"
#include <iostream>
using namespace std;



int  main(void)
{

    mySerial serial("/dev/ttyACM0",9600);
	unsigned char answer;

	
    // One Byte At the time
     serial.Send(1);
	serial.Receive(answer,1);
     serial.Send(1);
	serial.Receive(answer,1);
    // An array of byte
    //unsigned char  dataArray[] = { 142,0};
    //serial.Send(dataArray,sizeof(dataArray));
	//serial.Receive(answer,1);

    // Or a string
    //serial.Send("this is it\r\n");

	serial.Close();

    return 0;
}
