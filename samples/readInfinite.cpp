#include "rs232.hpp"
#include <iostream>

int main (int argc, char **argv) {
    std::vector<std::string> ports = sakurajin::getAvailablePorts();

    sakurajin::RS232 rs232_interface{ports,sakurajin::baud9600};
    if(!rs232_interface.IsAvailable()){
        std::cerr << "No serial port is available!" << std::endl;
        return -1;
    }

    std::cout << "Available serial ports:" << std::endl;
    auto devCount = rs232_interface.getDeviceCount();
    for(size_t i = 0; i < devCount; i++){
        if(rs232_interface.IsAvailable(i)){
            std::cout << rs232_interface.GetDeviceName(i) << std::endl;
        }
    }

    while (true){
        auto [nextC, error] = rs232_interface.ReadNextChar();
        if(error < 0){
            std::cerr << "Error while reading the buffer" << std::endl;
            break;
        }
        std::cout << nextC << std::flush;
    }

}
