#include "rs232.hpp"

void sakurajin::RS232::Print(const std::string& text, std::ostream& errorStream) {
    if (rs232Devices.empty()) {
        return;
    }
    auto transferDevice = rs232Devices[currentDevice];
    if (transferDevice->getConnectionStatus() != sakurajin::connectionStatus::connected) {
        errorStream << "device is not connected";
        return;
    }

    for (auto c : text) {
        if (transferDevice->writeRawData(&c, 1) < 1) {
            errorStream << "error writing [" << c << "] to serial port";
        }
    }
}

std::shared_ptr<sakurajin::RS232_native> sakurajin::RS232::getNativeDevice(size_t index) const {
    if (rs232Devices.empty()) {
        return nullptr;
    }
    if (index < rs232Devices.size()) {
        return rs232Devices[index];
    }
    return rs232Devices[currentDevice];
}

size_t sakurajin::RS232::getDeviceCount() const {
    return rs232Devices.size();
}

bool sakurajin::RS232::IsAvailable(size_t index) const {
    auto dev = getNativeDevice(index);
    if (dev == nullptr) {
        return false;
    }

    return dev->getConnectionStatus() == sakurajin::connectionStatus::connected;
}

std::string_view sakurajin::RS232::GetDeviceName(size_t index) const {
    auto dev = getNativeDevice(index);
    return dev == nullptr ? "" : dev->getDeviceName();
}

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
}

sakurajin::RS232::RS232(const std::string& deviceName, sakurajin::Baudrate Rate, std::ostream& errorStream)
    : RS232(std::vector<std::string>{deviceName}, Rate, errorStream) {}

[[maybe_unused]]
sakurajin::RS232::RS232(sakurajin::Baudrate Rate, std::ostream& errorStream)
    : RS232(sakurajin::getAvailablePorts(), Rate, errorStream) {}

sakurajin::RS232::~RS232() {
    DisconnectAll();
}

void sakurajin::RS232::Close() {
    DisconnectAll();
}

bool sakurajin::RS232::Connect() {
    if (rs232Devices.empty()) {
        return false;
    }

    // return early if the current device is already connected
    if (rs232Devices[currentDevice]->getConnectionStatus() == sakurajin::connectionStatus::connected) {
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

std::tuple<unsigned char, int> sakurajin::RS232::ReadNextChar() {
    return ReadNextChar(std::chrono::microseconds(1), true);
}

std::tuple<std::string, int> sakurajin::RS232::ReadNextMessage() {
    return ReadNextMessage(std::chrono::microseconds(1), true);
}

std::tuple<std::string, int> sakurajin::RS232::ReadUntil(const std::vector<unsigned char>& conditions) {
    return ReadUntil(conditions, std::chrono::microseconds(1), true);
}

bool sakurajin::RS232::IsCTSEnabled() const {
    auto nativeDevice = getNativeDevice();
    if (nativeDevice == nullptr) {
        return false;
    }
    return nativeDevice->checkForFlag(REQUEST_TO_SEND);
}
