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
    return dev == nullptr ? false : dev->getConnectionStatus() == sakurajin::connectionStatus::connected;
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
        rs232Devices.emplace_back(std::make_shared<sakurajin::RS232_native>(deviceName, baudrate, errorStream));
    }

    // find the first connected device or a device that could be connected to but is not connected yet
    for (size_t i = 0; i < rs232Devices.size(); i++) {
        switch (rs232Devices[i]->getConnectionStatus()) {
            case sakurajin::connectionStatus::connected:
                currentDevice = i;
                return;
            case sakurajin::connectionStatus::disconnected:
                currentDevice = i;
                break;
            case sakurajin::connectionStatus::otherError:
                currentDevice = i;
                break;
            default:
                break;
        }
    }
}

sakurajin::RS232::RS232(const std::string& deviceName, sakurajin::Baudrate Rate, std::ostream& errorStream)
    : RS232(std::vector<std::string>{deviceName}, Rate, errorStream) {}

sakurajin::RS232::RS232(sakurajin::Baudrate Rate, std::ostream& errorStream)
    : RS232(sakurajin::getAvailablePorts(), Rate, errorStream) {}

sakurajin::RS232::~RS232() {
    Close();
}

void sakurajin::RS232::Close() {
    for (auto device : rs232Devices) {
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
