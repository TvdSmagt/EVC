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
	}
return 0;
}
