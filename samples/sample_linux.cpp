#include "rs232.hpp"
#include <iostream>

using namespace kfx;

int main () {
    const std::string serialPort = "/dev/ttyUSB0";
    
    RS232 h{serialPort,baud9600};

    if(!h.IsAvailable()){
        std::cerr << "Serial port " << h.GetDeviceName() << " is not available!" << std::endl;
        return -1;
    }

    // Sending a Character
    h.Write('A');

    // Reciving a buffer
    unsigned char buf[30];
    h.Read(buf,22);
    std::cout << buf << std::endl;

    // Sending a buffer
    h.Print("Hello!");
    h.Read(buf,22);
    std::cout << buf << std::endl;

    // Closing port
    std::cout << "\nClosing Serialport ..." << std::endl;
    h.Close();

    return 0;
}
