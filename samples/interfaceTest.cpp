#include "rs232.hpp"
#include <iostream>

using namespace sakurajin;

void printNextBuffer(RS232& h){
    auto buffer = h.ReadNextMessage();
    
    if(std::get<1>(buffer) < 0){
        std::cerr << "Error while reading the buffer" << std::endl;
    }else{
        std::cout << std::get<0>(buffer) ;
    }
}

int main (int argc, char **argv) {

    // get the port form the parameter or set the default value
    std::string serialPort = "";

    if(argc == 2){
        serialPort = argv[1];
    }else{
        #if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
            serialPort = "/dev/ttyUSB0";
        #else
            serialPort = "\\\\.\\COM3";
        #endif
    }
    
    // create the rs232 handle and check if the connection worked
    RS232 h{serialPort,baud19200};

    if(!h.IsAvailable()){
        std::cerr << "Serial port " << h.GetDeviceName() << " is not available!" << std::endl;
        return -1;
    }

    // start the communication
    h.Print("start");

    // read the first message
    printNextBuffer(h);
    
    //send a message to the arduino
    h.Print("Hello from the other side!");

    //get the response from the arduino
    printNextBuffer(h);

    // Closing port
    std::cout << "\nClosing Serialport ..." << std::endl;
    h.Close();

    return 0;
}

