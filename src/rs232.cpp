#include "rs232.hpp"

// Sends a string to serial port till finding a '\0'
void sakurajin::RS232::Print(const std::string& text) {
    auto transferDevice = rs232Devices[currentDevice];
    if (transferDevice->getConnectionStatus() != sakurajin::connectionStatus::connected) {
        return;
    }

    for (unsigned char c : text) {
        if (transferDevice->writeRawData(&c, 1) < 1) {
            throw std::runtime_error("unable to write to serial port");
        }
    }
}

bool sakurajin::RS232::IsAvailable() const {
    return rs232Devices[currentDevice]->getConnectionStatus() == sakurajin::connectionStatus::connected;
}

std::string_view sakurajin::RS232::GetDeviceName() const {
    return rs232Devices[currentDevice]->getDeviceName();
}

sakurajin::RS232::RS232(const std::string& deviceName, sakurajin::Baudrate Rate) {
    rs232Devices.push_back(std::make_shared<RS232_native>(deviceName, Rate));
    currentDevice = 0;
}

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
