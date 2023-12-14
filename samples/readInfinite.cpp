#include "rs232.hpp"

constexpr const bool regexExample = true;

/**
 * @brief This sample shows how to read from a serial port until the end of the buffer is reached.
 *
 * It first tries to get a list of available ports and then tries to connect to the first one.
 */
int main (int argc, char **argv) {
    //try to connect to all potentially available ports with the baudrate 9600
    sakurajin::RS232 rs232_interface{sakurajin::baud9600};
    if(!rs232_interface.IsAvailable()){
        std::cerr << "No serial port is available!" << std::endl;
        return -1;
    }

    //iterate over all available ports and print all connected to the console
    std::cout << "Available serial ports:" << std::endl;
    auto devCount = rs232_interface.getDeviceCount();
    for(size_t i = 0; i < devCount; i++){
        auto deviceI = rs232_interface.getNativeDevice(i);
        if(deviceI == nullptr){
            continue;
        }

        if(deviceI->getConnectionStatus() == sakurajin::connectionStatus::connected) {
            std::cout << deviceI->getDeviceName() << " (connected)" << std::endl;
        }
    }

    if(regexExample) {
        //setup the regex pattern
        //here the pattern is X followed by 7 non whitespace characters followed by Y
        std::regex pattern{"X\\S{7}Y"};

        while (true) {
            //do a small delay to prevent too much locking
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            //get the first match of the pattern in the read buffer
            auto result = rs232_interface.retrieveFirstMatch(pattern);

            //don't output anything if there is no match
            if (result.empty()) {
                continue;
            }

            //output the result to the console
            std::cout << result << std::endl;
        }
    }else{
        while(true){
            //do a small delay to prevent too much locking
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            //read the string and save the result in a variable
            auto readString = rs232_interface.retrieveReadBuffer();

            //output the string to the console
            std::cout << readString << std::flush;
        }
    }

}
