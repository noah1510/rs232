#include "rs232_native.hpp"

#include "windows.h"

#include <sstream>
#include <algorithm>

inline HANDLE& getCport(void* portHandle){
    return *static_cast<HANDLE*>(portHandle);
}

inline DCB& getDCB(void* DCBHandle){
    return *static_cast<DCB*>(DCBHandle);
}

sakurajin::connectionStatus sakurajin::RS232_native::connect(sakurajin::Baudrate baudrate, std::ostream& error_stream) {
    if(connStatus == connectionStatus::connected){
        return connStatus;
    }

    std::stringstream baudr_conf;
    baudr_conf << "baud=" << baudrate << " data=8 parity=N stop=1";

    portHandle = static_cast<void*>(new HANDLE{});
    getCport(portHandle) = CreateFileA(
        devname.c_str(),
        GENERIC_READ|GENERIC_WRITE,
        0,                          /* no share  */
        NULL,                       /* no security */
        OPEN_EXISTING,
        0,                          /* no threads */
        NULL     /* no templates */
    );

    if(getCport(portHandle) == INVALID_HANDLE_VALUE){
        error_stream << "unable to open comport:" << GetLastError();
        connStatus = connectionStatus::portNotFound;
        return connStatus;
    }

    portConfig = static_cast<void*>(new DCB{});
    getDCB(portConfig).DCBlength = sizeof(DCB);

    if(!BuildCommDCBA(baudr_conf.str().c_str(), &getDCB(portConfig))){
        error_stream << "unable to set comport dcb settings";
        CloseHandle(getCport(portHandle));
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    if(!SetCommState(getCport(portHandle), &getDCB(portConfig))){
        error_stream << "unable to set comport cfg settings";
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

    if(!SetCommTimeouts(getCport(portHandle), &Cptimeouts)){
        error_stream << "unable to set comport time-out settings";
        CloseHandle(getCport(portHandle));
        connStatus = connectionStatus::otherError;
        return connStatus;
    }

    connStatus = connectionStatus::connected;
    return connStatus;
}

void sakurajin::RS232_native::disconnect() noexcept {
    if(connStatus != connectionStatus::connected){
        return;
    }

    CloseHandle(getCport(portHandle));

    delete &getCport(portHandle);
    delete &getDCB(portConfig);
    portHandle = nullptr;
    portConfig = nullptr;

    connStatus = connectionStatus::disconnected;
}

int sakurajin::RS232_native::readRawData(unsigned char* data_location, int length){
    if (connStatus != connectionStatus::connected){
        return -1;
    }

    int n = 0;
    length = std::clamp(length, 0, 4096);

  /* added the void pointer cast, otherwise gcc will complain about */
  /* "warning: dereferencing type-punned pointer will break strict aliasing rules" */
    ReadFile(getCport(portHandle), data_location, length, (LPDWORD)((void *)&n), NULL);
    return n;
}

int sakurajin::RS232_native::writeRawData(unsigned char* data_location, int length){
    if (connStatus != connectionStatus::connected){
        return -1;
    }

    int n = 0;

    if(WriteFile(getCport(portHandle), data_location, length, (LPDWORD)((void *)&n), NULL)){
        return n;
    }

    return -1;
}

int sakurajin::RS232_native::retrieveFlags() {
    if (connStatus != connectionStatus::connected){
        return -1;
    }

    DWORD status;
    if(!GetCommModemStatus(getCport(portHandle), &status)){
        return -1;
    }

    return static_cast<int>(status);
}
