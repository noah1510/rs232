#include "rs232.hpp"

using namespace std::literals;

// constructors and destructors
sakurajin::RS232::RS232(const std::vector<std::string>& deviceNames, sakurajin::Baudrate baudrate, std::ostream& errorStream) {
    if (deviceNames.empty()) {
        errorStream << "No device name was given. Creating empty RS232 object.";
        return;
    }

    // add all devices to the list
    rs232Devices.reserve(deviceNames.size());
    for (const auto& deviceName : deviceNames) {
        try {
            rs232Devices.emplace_back(std::make_shared<sakurajin::RS232_native>(deviceName, baudrate, errorStream));
        } catch (...) {
            for (auto& device : rs232Devices) {
                device->disconnect();
            }
            rs232Devices.clear();
            std::throw_with_nested(std::runtime_error("error constructing: " + deviceName));
        }
    }

    Connect();

    // start the work thread
    workThread = std::async(std::launch::async, [this]() {
        while (!stopThread) {
            work();
        }
    });
}

sakurajin::RS232::RS232(const std::string& deviceName, sakurajin::Baudrate Rate, std::ostream& errorStream)
    : RS232(std::vector<std::string>{deviceName}, Rate, errorStream) {}

[[maybe_unused]]
sakurajin::RS232::RS232(sakurajin::Baudrate Rate, std::ostream& errorStream)
    : RS232(sakurajin::getAvailablePorts(), Rate, errorStream) {}

sakurajin::RS232::~RS232() {
    // correctly stop the work thread before disconnecting everything
    stopThread = true;
    if (workThread.valid()) {
        workThread.wait();
    }

    DisconnectAll();
}

// connection functions
bool sakurajin::RS232::Connect() {
    if (rs232Devices.empty()) {
        return false;
    }

    // return early if the current device is already connected
    if (getCurrentDevice()->getConnectionStatus() == sakurajin::connectionStatus::connected) {
        return true;
    }

    // connect all devices
    for (auto& device : rs232Devices) {
        device->connect();
    }

    // find the first connected device or a device that can be connected to but is disconnected at the moment
    for (size_t i = 0; i < rs232Devices.size(); i++) {
        switch (rs232Devices[i]->getConnectionStatus()) {
            case sakurajin::connectionStatus::connected:
                currentDevice = i;
                return true;
            case sakurajin::connectionStatus::disconnected:
                currentDevice = i;
            case sakurajin::connectionStatus::otherError:
            case sakurajin::connectionStatus::portNotFound:
                break;
            default:
                throw std::runtime_error("unknown connection status");
        }
    }

    // no device was connected successfully so return false
    return false;
}

void sakurajin::RS232::DisconnectAll() {
    for (const auto& device : rs232Devices) {
        device->disconnect();
    }
}

// the work function
void sakurajin::RS232::work() {
    if (rs232Devices.empty()) {
        // if there are no devices, wait for 100ms
        // the delay is to prevent the thread from spinning
        // since it is unlikely that a device will be added the delay is higher than for the other cases
        std::this_thread::sleep_for(100ms);
        return;
    }

    auto transferDevice = getCurrentDevice();
    if (transferDevice->getConnectionStatus() != sakurajin::connectionStatus::connected) {
        // it is more likely that a device will be connected than that a device will be added
        // because of this the sleep duration is lower
        std::this_thread::sleep_for(1ms);
        return;
    }

    // if there is something to write to the device, write it
    if (writeBufferHasData) {
        // lock the mutex to prevent the buffer from being changed while it is being moved
        writeBufferMutex.lock();
        auto localWriteBuffer = std::move(writeBuffer);
        writeBufferHasData    = false;
        writeBufferMutex.unlock();

        // write the data
        for (auto c : localWriteBuffer) {
            if (transferDevice->writeRawData(&c, 1) < 1) {
                std::cerr << "error writing [" << c << "] to serial port";
            }
        }
    }

    // read the data
    char    IOBuf    = '\0';
    int64_t dataSize = transferDevice->readRawData(&IOBuf, 1);
    if (dataSize > 0) {
        // if there is data, add it to the buffer and set the hasData flag
        readBufferMutex.lock();
        if (readBufferHasData) {
            readBuffer.append(1, IOBuf);
        } else {
            readBuffer = std::string(1, IOBuf);
            readBuffer.reserve(10);
            readBufferHasData = true;
        }
        readBufferMutex.unlock();
    }
}

// io functions
void sakurajin::RS232::Print(const std::string& text) {
    std::scoped_lock lock(writeBufferMutex);

    if (writeBufferHasData) {
        writeBuffer.append(text);
    } else {
        writeBuffer        = text;
        writeBufferHasData = true;
    }
}

std::string&& sakurajin::RS232::retrieveReadBuffer() {
    if (!readBufferHasData) {
        return std::string();
    }

    std::scoped_lock lock(readBufferMutex);

    readBufferHasData = false;
    return std::move(readBuffer);
}

std::string&& sakurajin::RS232::retrieveFirstMatch(const std::regex& pattern) {
    if (!readBufferHasData) {
        return std::string();
    }

    std::scoped_lock lock(readBufferMutex);
    std::smatch      s_match_result;
    std::regex_search(readBuffer, s_match_result, pattern);
    if (s_match_result.empty()) {
        return std::string();
    }

    auto result = s_match_result.str();
    readBuffer  = s_match_result.suffix();
    return std::move(result);
}

// device access functions
std::shared_ptr<sakurajin::RS232_native> sakurajin::RS232::getNativeDevice(size_t index) const {
    if (rs232Devices.empty()) {
        return nullptr;
    }
    if (index < rs232Devices.size()) {
        return rs232Devices[index];
    }
    return rs232Devices[currentDevice];
}

std::shared_ptr<sakurajin::RS232_native> sakurajin::RS232::getCurrentDevice() const {
    return getNativeDevice(currentDevice);
}
bool sakurajin::RS232::IsAvailable() const {
    return getCurrentDevice()->getConnectionStatus() == sakurajin::connectionStatus::connected;
}

size_t sakurajin::RS232::getDeviceCount() const {
    return rs232Devices.size();
}

// deprecated functions
void sakurajin::RS232::Close() {
    DisconnectAll();
}

std::string_view sakurajin::RS232::GetDeviceName(size_t index) const {
    auto dev = getNativeDevice(index);
    return dev == nullptr ? "" : dev->getDeviceName();
}

bool sakurajin::RS232::IsCTSEnabled() const {
    auto nativeDevice = getCurrentDevice();
    if (nativeDevice == nullptr) {
        return false;
    }
    return nativeDevice->checkForFlag(REQUEST_TO_SEND);
}

bool sakurajin::RS232::IsAvailable(size_t index) const {
    auto dev = getNativeDevice(index);
    if (dev == nullptr) {
        return false;
    }

    return dev->getConnectionStatus() == sakurajin::connectionStatus::connected;
}
