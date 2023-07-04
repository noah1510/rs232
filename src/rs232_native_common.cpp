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
