#include "rs232.h"
#include <iostream>

using namespace std;
using namespace kfx;

int main () {
    char serialPort[] = "/dev/ttyUSB0";
    int baudrate = 9600;
    
    
    RS232 h{serialPort,baudrate};
    if(!h.IsAvailable()){
        cout << "Serial port %s is not available" << serialPort << endl;
    }

    // Sending a Character
    h.Write('A');

    // Reciving a buffer
    unsigned char buf[30];
    h.Read(buf,22);
    cout << buf << endl;

    // Sending a buffer
    h.Print("Hello!");
    h.Read(buf,22);
    cout << buf << endl;

    // Closing port
    cout << "\nClosing Serialport ..." << endl;
    h.Close();

    return 0;
}
