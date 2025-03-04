#include "rs232_native.hpp"

#include "windows.h"

#include <codecvt>
#include <locale>

static inline std::string wstrToStr(const std::wstring& wideStr) noexcept {
    if (wideStr.empty()) {
        return "";
    }
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize <= 0) {
        return "";
    }

    // Create a buffer to store the converted string
    std::string narrowStr(bufferSize, '\0');

    // Perform the conversion
    auto conversion = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &narrowStr[0], bufferSize, nullptr, nullptr);
    return (conversion > 0) ? narrowStr : "";
}

inline HANDLE& getCport(void* portHandle) noexcept {
    return *static_cast<HANDLE*>(portHandle);
}

inline DCB& getDCB(void* DCBHandle) noexcept {
    return *static_cast<DCB*>(DCBHandle);
}

std::vector<std::string> sakurajin::getMatchingPorts(const std::regex& pattern) noexcept {
    std::vector<std::string> allPorts;
    wchar_t                  lpTargetPath[5000];

    for (uint8_t i = 0; i < 255; i++) {
        std::wstringstream wss;
        wss << "\\\\.\\COM" << i;
        DWORD res = QueryDosDeviceW(wss.str().c_str(), lpTargetPath, 5000);

        // Test the return value and error if any
        if (res != 0) {
            std::string narrowString = wstrToStr(wss.str());
            if (std::regex_match(narrowString, pattern)) {
                allPorts.push_back(narrowString);
            }
            std::cout << narrowString << ": " << lpTargetPath << std::endl;
        }
    }

    allPorts.shrink_to_fit();
    return allPorts;
}

sakurajin::connectionStatus sakurajin::RS232_native::connect(std::ostream& error_stream) noexcept {
    if (connStatus == connectionStatus::connected) {
        return connStatus;
    }

    std::scoped_lock lock{dataAccessMutex};

    std::stringstream baudr_conf;
    baudr_conf << "baud=" << baudrate << " data=8 parity=N stop=1";

    portHandle           = static_cast<void*>(new HANDLE{});
    getCport(portHandle) = CreateFileA(devname.c_str(),
                                       GENERIC_READ | GENERIC_WRITE,
                                       0,    /* no share  */
                                       NULL, /* no security */
                                       OPEN_EXISTING,
                                       0,   /* no threads */
                                       NULL /* no templates */
    );

    if (getCport(portHandle) == INVALID_HANDLE_VALUE) {
        error_stream << "unable to open comport " << devname << " message:" << GetLastError() << std::endl;
        connStatus = connectionStatus::portNotFound;
        return connStatus;
    }

    portConfig                   = static_cast<void*>(new DCB{});
    getDCB(portConfig).DCBlength = sizeof(DCB);

    if (!BuildCommDCBA(baudr_conf.str().c_str(), &getDCB(portConfig))) {
        error_stream << "unable to set comport dcb settings for " << devname << std::endl;
        CloseHandle(getCport(portHandle));
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    if (!SetCommState(getCport(portHandle), &getDCB(portConfig))) {
        error_stream << "unable to set comport cfg settings for " << devname << std::endl;
        CloseHandle(getCport(portHandle));
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    COMMTIMEOUTS Cptimeouts;

    Cptimeouts.ReadIntervalTimeout         = MAXDWORD;
    Cptimeouts.ReadTotalTimeoutMultiplier  = 0;
    Cptimeouts.ReadTotalTimeoutConstant    = 0;
    Cptimeouts.WriteTotalTimeoutMultiplier = 0;
    Cptimeouts.WriteTotalTimeoutConstant   = 0;

    if (!SetCommTimeouts(getCport(portHandle), &Cptimeouts)) {
        error_stream << "unable to set comport time-out settings for " << devname << std::endl;
        CloseHandle(getCport(portHandle));
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    connStatus = connectionStatus::connected;
    return connStatus;
}

void sakurajin::RS232_native::disconnect() noexcept {
    if (connStatus != connectionStatus::connected) {
        return;
    }

    std::scoped_lock lock{dataAccessMutex};

    CloseHandle(getCport(portHandle));

    delete &getCport(portHandle);
    delete &getDCB(portConfig);
    portHandle = nullptr;
    portConfig = nullptr;

    connStatus = connectionStatus::disconnected;
}

int64_t sakurajin::RS232_native::readRawData(char* data_location, int length, bool block) noexcept {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    return callWithOptionalLock<int64_t>(
        [this, data_location, length]() {
            int  n         = 0;
            auto local_len = std::clamp(length, 0, 4096);

            auto success = ReadFile(getCport(portHandle), data_location, local_len, (LPDWORD)((void*)&n), NULL);
            return (int64_t)(success ? n : -1);
        },
        block);
}

int64_t sakurajin::RS232_native::writeRawData(char* data_location, int length, bool block) noexcept {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    return callWithOptionalLock<int64_t>(
        [this, data_location, length]() {
            int  n         = 0;
            auto local_len = std::clamp(length, 0, 4096);

            auto success = WriteFile(getCport(portHandle), data_location, local_len, (LPDWORD)((void*)&n), NULL);
            return (int64_t)(success ? n : -1);
        },
        block);
}

int64_t sakurajin::RS232_native::retrieveFlags(bool block) noexcept {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    return callWithOptionalLock<int64_t>(
        [this]() {
            DWORD flags;
            if (!GetCommModemStatus(getCport(portHandle), &flags)) {
                return (int64_t)(-1);
            }
            return (int64_t)flags;
        },
        block);
}
