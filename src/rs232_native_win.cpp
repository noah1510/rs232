#include "rs232_native.hpp"

#include "windows.h"

#include <algorithm>
#include <sstream>
#include <locale>
#include <codecvt>

static inline std::string wstrToStr(const std::wstring &wstr){
    if( wstr.empty() ) return "";
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

inline HANDLE& getCport(void* portHandle) {
    return *static_cast<HANDLE*>(portHandle);
}

inline DCB& getDCB(void* DCBHandle) {
    return *static_cast<DCB*>(DCBHandle);
}

std::vector<std::string> sakurajin::getMatchingPorts(std::regex pattern) {
    std::vector<std::string> allPorts;
    wchar_t lpTargetPath[5000];

    for (uint8_t i = 0; i < 255; i++) {
        std::wstringstream wss;
        wss << "\\\\.\\COM" << i;
        DWORD res = QueryDosDeviceW(wss.str().c_str(), lpTargetPath, 5000);

        // Test the return value and error if any
        if (res != 0) {
            std::string narrowString = wstrToStr(wss.str());
            if(std::regex_match(narrowString, pattern)){
                allPorts.push_back(narrowString);
            }
            std::cout << narrowString << ": " << lpTargetPath << std::endl;
        }
    }

    allPorts.shrink_to_fit();
    return allPorts;
}

sakurajin::connectionStatus sakurajin::RS232_native::connect(sakurajin::Baudrate baudrate, std::ostream& error_stream) {
    if (connStatus == connectionStatus::connected) {
        return connStatus;
    }

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

    CloseHandle(getCport(portHandle));

    delete &getCport(portHandle);
    delete &getDCB(portConfig);
    portHandle = nullptr;
    portConfig = nullptr;

    connStatus = connectionStatus::disconnected;
}

int sakurajin::RS232_native::readRawData(char* data_location, int length) {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    int n  = 0;
    length = std::clamp(length, 0, 4096);

    /* added the void pointer cast, otherwise gcc will complain about */
    /* "warning: dereferencing type-punned pointer will break strict aliasing rules" */
    ReadFile(getCport(portHandle), data_location, length, (LPDWORD)((void*)&n), NULL);
    return n;
}

int sakurajin::RS232_native::writeRawData(char* data_location, int length) {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    int n = 0;

    if (WriteFile(getCport(portHandle), data_location, length, (LPDWORD)((void*)&n), NULL)) {
        return n;
    }

    return -1;
}

int sakurajin::RS232_native::retrieveFlags() {
    if (connStatus != connectionStatus::connected) {
        return -1;
    }

    DWORD status;
    if (!GetCommModemStatus(getCport(portHandle), &status)) {
        return -1;
    }

    return static_cast<int>(status);
}
