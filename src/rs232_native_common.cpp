#include <array>
#include <utility>

#include "rs232_native.hpp"

using namespace std::literals;

sakurajin::RS232_native::RS232_native(std::string deviceName, Baudrate _baudrate, std::ostream& error_stream)
    : devname(std::move(deviceName)) {
    baudrate   = _baudrate;
    connStatus = connect(error_stream);
}

sakurajin::RS232_native::~RS232_native() {
    disconnect();
}

bool sakurajin::RS232_native::changeBaudrate(sakurajin::Baudrate Rate, std::ostream& error_stream) noexcept {
    disconnect();
    baudrate = Rate;
    return connect(error_stream) == connectionStatus::connected;
}

sakurajin::connectionStatus sakurajin::RS232_native::getConnectionStatus() noexcept {
    return connStatus;
}

std::string_view sakurajin::RS232_native::getDeviceName() const noexcept {
    return devname;
}

bool sakurajin::RS232_native::checkForFlag(sakurajin::portStatusFlags flag, bool block) noexcept {
    auto flags = retrieveFlags(block);
    if (flags < 0) {
        return false;
    }
    return (flags & static_cast<int64_t>(flag)) != 0;
}

std::vector<std::string> sakurajin::getAvailablePorts() noexcept {
    std::vector<std::string> allPorts;

    // All the regex patterns to check for available ports
    // These are the patterns across every platform that I could find
    // The filtering for the os is done in the getMatchingPorts function
    using rgx = std::regex;
    const std::array<std::regex, 7> patters{rgx("/dev/ttyUSB[0-9]+"),
                                            rgx("/dev/ttyACM[0-9]+"),
                                            rgx("\\./COM[0-9]+"),
                                            rgx("/dev/cu\\.[a-zA-Z0-9]+"),
                                            rgx("/dev/cuaU[0-9]+"),
                                            rgx("/dev/tty.usbmodem[0-9]+"),
                                            rgx("/dev/tty.usbserial[0-9]+")};

    for (const auto& pattern : patters) {
        // get all matches for the regex pattern and continue if there are none
        auto matches = sakurajin::getMatchingPorts(pattern);
        if (matches.empty()) {
            continue;
        }

        // remove duplicates
        for (const auto& port : allPorts) {
            for (auto matchIT = matches.begin(); matchIT != matches.end(); matchIT++) {
                if (*matchIT == port) {
                    matchIT = matches.erase(matchIT);
                }
            }
        }

        // add all non duplicates to the list
        allPorts.insert(allPorts.end(), matches.begin(), matches.end());
    }

    return allPorts;
}

std::tuple<unsigned char, int> sakurajin::native::ReadNextChar(const std::shared_ptr<RS232_native>& transferDevice) {
    return sakurajin::native::ReadNextChar(transferDevice, std::chrono::microseconds(1), true);
}

std::tuple<std::string, int> sakurajin::native::ReadNextMessage(const std::shared_ptr<RS232_native>& transferDevice) {
    return sakurajin::native::ReadNextMessage(transferDevice, 1us, true);
}

std::tuple<std::string, int> sakurajin::native::ReadUntil(const std::shared_ptr<RS232_native>& transferDevice,
                                                          const std::vector<unsigned char>&    conditions) {
    return sakurajin::native::ReadUntil(transferDevice, conditions, 1us, true);
}

int sakurajin::native::Print(const std::shared_ptr<RS232_native>& transferDevice, const std::string& text) {

    if (transferDevice == nullptr) {
        return -1;
    }

    if (transferDevice->getConnectionStatus() != sakurajin::connectionStatus::connected) {
        return -2;
    }

    for (auto c : text) {
        // retry each character until it is written
        int64_t writeRes;
        do {
            writeRes = transferDevice->writeRawData(&c, 1);

            // if the connection was lost while writing, return
            if (transferDevice->getConnectionStatus() != sakurajin::connectionStatus::connected) {
                return -3;
            }
        } while (writeRes < 1);
    }

    return 0;
}
