#include "rs232_native.hpp"

#include <algorithm>
#include <filesystem>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

inline int& getPort(void* portHandle) {
    return *static_cast<int*>(portHandle);
}

inline struct termios& getTermios(void* termiosHandle) {
    return *static_cast<termios*>(termiosHandle);
}

sakurajin::connectionStatus sakurajin::RS232_native::connect(Baudrate baudrate, std::ostream& error_stream) {
    if (connStatus == connectionStatus::connected) {
        return connStatus;
    }

    // convert the baudrate to int
    int baudr = baudrate;

    // check if the file for the port exists
    std::filesystem::path devicePath = devname;
    if (devicePath.is_relative()) {
        devicePath = "/dev" / devicePath;
    }

    if (!std::filesystem::exists(devicePath)) {
        error_stream << "device " << devicePath << " does not exist";
        connStatus = connectionStatus::portNotFound;
        return connStatus;
    }

    portHandle          = static_cast<void*>(new int{});
    getPort(portHandle) = open(devicePath.string().c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (getPort(portHandle) < 0) {
        error_stream << "unable to open comport ";
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    portConfig = static_cast<void*>(new termios{});
    int error  = tcgetattr(getPort(portHandle), &getTermios(portConfig));
    if (error < 0) {
        close(getPort(portHandle));
        error_stream << "unable to read port settings";
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    struct termios nps {};

    nps.c_cflag     = baudr | CS8 | CLOCAL | CREAD;
    nps.c_iflag     = IGNPAR;
    nps.c_oflag     = 0;
    nps.c_lflag     = 0;
    nps.c_cc[VMIN]  = 0; /* block until n bytes are received */
    nps.c_cc[VTIME] = 0; /* block until a timer expires (n * 100 mSec.) */

    error = tcsetattr(getPort(portHandle), TCSANOW, &nps);
    if (error < 0) {
        close(getPort(portHandle));
        error_stream << "unable to adjust port settings " << std::endl;
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    connStatus = connectionStatus::connected;
    return connStatus;
}

int sakurajin::RS232_native::readRawData(unsigned char* data_location, int length) {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    int limit =
#ifndef __STRICT_ANSI__ /* __STRICT_ANSI__ is defined when the -ansi option is used for gcc */
        (int)SSIZE_MAX; /* SSIZE_MAX is defined in limits.h */
#else
        4096;
#endif

    length = std::clamp(length, 0, limit);

    return read(getPort(portHandle), data_location, length);
}

int sakurajin::RS232_native::writeRawData(unsigned char* data_location, int length) {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    return write(getPort(portHandle), data_location, length);
}

void sakurajin::RS232_native::disconnect() noexcept {
    if (connStatus != connectionStatus::connected) {
        return;
    }
    connStatus = connectionStatus::disconnected;

    close(getPort(portHandle));
    tcsetattr(getPort(portHandle), TCSANOW, &getTermios(portConfig));

    delete &getPort(portHandle);
    delete &getTermios(portConfig);
    portHandle = nullptr;
    portConfig = nullptr;
}

int sakurajin::RS232_native::retrieveFlags() {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    int status;
    if(ioctl(getPort(portHandle), TIOCMGET, &status) < 0){
        return -1;
    }

    return status;
}
