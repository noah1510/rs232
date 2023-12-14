#include "rs232_native.hpp"

#include <climits>

#include <fcntl.h>
#include <unistd.h>

inline int& getPort(void* portHandle) {
    return *static_cast<int*>(portHandle);
}

inline struct termios& getTermios(void* termiosHandle) {
    return *static_cast<termios*>(termiosHandle);
}

std::vector<std::string> sakurajin::getMatchingPorts(const std::regex& pattern) {

    std::vector<std::string> allPorts;

    // check every file in /dev for a match with the given regex pattern
    // If it matches, add it to the list of available ports
    for (const auto& entry : std::filesystem::directory_iterator("/dev")) {
        auto filename = entry.path().string();
        if (!std::regex_match(filename, pattern)) {
            continue;
        }
        allPorts.push_back(filename);
    }

    return allPorts;
}

sakurajin::connectionStatus sakurajin::RS232_native::connect(Baudrate baudrate, std::ostream& error_stream) {
    if (connStatus == connectionStatus::connected) {
        return connStatus;
    }

    //make sure no read or write operation is performed while the port is being opened
    std::scoped_lock lock{dataAccessMutex};

    // convert the baudrate to int
    int baudr = baudrate;

    // check if the file for the port exists
    std::filesystem::path devicePath = devname;
    if (devicePath.is_relative()) {
        devicePath = "/dev" / devicePath;
    }

    //if the file does not exist, return an error
    if (!std::filesystem::exists(devicePath)) {
        error_stream << "device " << devicePath << " does not exist" << std::endl;
        connStatus = connectionStatus::portNotFound;
        return connStatus;
    }

    //open the port and return an error if that fails
    portHandle          = static_cast<void*>(new int{});
    getPort(portHandle) = open(devicePath.string().c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (getPort(portHandle) < 0) {
        error_stream << "unable to open device port " << devicePath << std::endl;
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    //get the current port settings and return an error if that fails
    portConfig = static_cast<void*>(new termios{});
    int error  = tcgetattr(getPort(portHandle), &getTermios(portConfig));
    if (error < 0) {
        close(getPort(portHandle));
        error_stream << "unable to read port settings for " << devicePath << std::endl;
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
        error_stream << "unable to adjust port settings for " << devicePath << std::endl;
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    connStatus = connectionStatus::connected;
    return connStatus;
}

ssize_t sakurajin::RS232_native::readRawData(char* data_location, int length, bool lock) {
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

    return callWithOptionalLock(lock, [this, data_location, length]() { return read(getPort(portHandle), data_location, length); });
}

ssize_t sakurajin::RS232_native::writeRawData(char* data_location, int length, bool lock) {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    return callWithOptionalLock(lock, [this, data_location, length]() { return write(getPort(portHandle), data_location, length); });
}

void sakurajin::RS232_native::disconnect() noexcept {
    if (connStatus != connectionStatus::connected) {
        return;
    }

    //lock the mutex to make sure the port is not accessed while it is being closed
    std::scoped_lock lock(dataAccessMutex);

    connStatus = connectionStatus::disconnected;

    //close the port handles
    close(getPort(portHandle));
    tcsetattr(getPort(portHandle), TCSANOW, &getTermios(portConfig));

    //free the memory and set the pointers to nullptr
    delete &getPort(portHandle);
    delete &getTermios(portConfig);
    portHandle = nullptr;
    portConfig = nullptr;
}

ssize_t sakurajin::RS232_native::retrieveFlags(bool block) {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    return callWithOptionalLock(block, [this]() {
        ssize_t status;
        if (ioctl(getPort(portHandle), TIOCMGET, &status) < 0) {
            return (ssize_t)(-1);
        }
        return status;
    });
}
