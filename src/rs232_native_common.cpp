#include "rs232_native.hpp"

sakurajin::RS232_native::RS232_native(const std::string& deviceName, Baudrate baudrate, std::ostream& error_stream)
    : devname(deviceName) {
    connStatus = connect(baudrate, error_stream);
}

sakurajin::RS232_native::~RS232_native() {
    disconnect();
}

sakurajin::connectionStatus sakurajin::RS232_native::getConnectionStatus() {
    return connStatus;
}

std::string_view sakurajin::RS232_native::getDeviceName() const {
    return devname;
}

bool sakurajin::RS232_native::checkForFlag(sakurajin::portStatusFlags flag) {
    auto flags = retrieveFlags();
    if (flags == -1) {
        return false;
    }
    return (flags & static_cast<int>(flag)) != 0;
}

std::vector<std::string> sakurajin::getAvailablePorts() {
    std::vector<std::string> allPorts;

    //All the regex patterns to check for available ports
    //These are the patterns across every platform that I could find
    //The filtering for the os is done in the getMatchingPorts function
    using rgx = std::regex;
    std::vector<std::regex> patters{
        rgx("/dev/ttyUSB[0-9]+"),
        rgx("/dev/ttyACM[0-9]+"),
        rgx("\\./COM[0-9]+"),
        rgx("/dev/cu\\.[a-zA-Z0-9]+"),
        rgx("/dev/cuaU[0-9]+"),
        rgx("/dev/tty.usbmodem[0-9]+"),
        rgx("/dev/tty.usbserial[0-9]+")
    };

    for (const auto& pattern : patters) {
        //get all matches for the regex pattern and continue if there are none
        auto matches = sakurajin::getMatchingPorts(pattern);
        if (matches.empty()) {continue;}

        //remove duplicates
        for (const auto& port: allPorts){
            for (auto matchIT = matches.begin(); matchIT != matches.end();){
                if (*matchIT == port){
                    matchIT = matches.erase(matchIT);
                } else {
                    matchIT++;
                }
            }
        }

        //add all non duplicates to the list
        allPorts.insert(allPorts.end(), matches.begin(), matches.end());
    }

    return allPorts;
}
