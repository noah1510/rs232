#include "rs232.hpp"
#include <iostream>

using namespace sakurajin;

int main () {
    const std::string serialPort = "/dev/ttyUSB0";
    
    RS232 h{serialPort,baud19200};

    if(!h.IsAvailable()){
        std::cerr << "Serial port " << h.GetDeviceName() << " is not available!" << std::endl;
        return -1;
    }

    // Reciving a buffer
    auto buffer = h.ReadNextMessage();
    
    if(std::get<1>(buffer) < 0){
        std::cerr << "Error while reading the buffer" << std::endl;
    }else{
        std::cout << std::get<0>(buffer) ;
    }
    
    
    std::cout << std::endl;

    // Closing port
    std::cout << "\nClosing Serialport ..." << std::endl;
    h.Close();

    return 0;
}

