/*
***************************************************************************
*
* Author: Frank Andre Moreno Vera
*
* Copyright (C) 2014 Frank Andre Moreno Vera
*
* frankmoreno1993@gmail.com
*
***************************************************************************
*/

#include "rs232.hpp"

sakurajin::RS232::RS232(const std::string& deviceName, Baudrate baudrate):devname(deviceName){

    std::ostringstream baudr_conf;
    baudr_conf << "baud=" << baudrate << " data=8 parity=N stop=1";

    Cport = CreateFileA(
        devname.c_str(),
          GENERIC_READ|GENERIC_WRITE,
          0,                          /* no share  */
          NULL,                       /* no security */
          OPEN_EXISTING,
          0,                          /* no threads */
          NULL     /* no templates */
    );                     

    if(Cport == INVALID_HANDLE_VALUE){
        std::cerr << "unable to open comport" << std::endl;
        return;
    }

    DCB port_settings;
    memset(&port_settings, 0, sizeof(port_settings));  /* clear the new struct  */
    port_settings.DCBlength = sizeof(port_settings);

    if(!BuildCommDCBA(baudr_conf.str().c_str(), &port_settings)){
        std::cerr << "unable to set comport dcb settings" << std::endl;
        CloseHandle(Cport);
        return;
    }

    if(!SetCommState(Cport, &port_settings)){
        std::cerr << "unable to set comport cfg settings" << std::endl;
        CloseHandle(Cport);
        return;
    }

    COMMTIMEOUTS Cptimeouts;

    Cptimeouts.ReadIntervalTimeout         = MAXDWORD;
    Cptimeouts.ReadTotalTimeoutMultiplier  = 0;
    Cptimeouts.ReadTotalTimeoutConstant    = 0;
    Cptimeouts.WriteTotalTimeoutMultiplier = 0;
    Cptimeouts.WriteTotalTimeoutConstant   = 0;

    if(!SetCommTimeouts(Cport, &Cptimeouts)){
        std::cerr << "unable to set comport time-out settings" << std::endl;
        CloseHandle(Cport);
        return;
    }

    available = true;

    return;
}

int sakurajin::RS232::Read(unsigned char *buf, int size){
    if (!available){
        return -1;
    }

    int n;

    size = std::clamp(size, 0, 4096);

  /* added the void pointer cast, otherwise gcc will complain about */
  /* "warning: dereferencing type-punned pointer will break strict aliasing rules" */

    ReadFile(Cport, buf, size, (LPDWORD)((void *)&n), NULL);

    return n;
}

int sakurajin::RS232::Write(unsigned char * buf, int size){
    if (!available){
        return -1;
    }

    int n;

    if(WriteFile(Cport, buf, size, (LPDWORD)((void *)&n), NULL)){
      return n;
    }

    return -1;
}

void sakurajin::RS232::Close(){
    available = false;
    CloseHandle(Cport);
}

int sakurajin::RS232::IsCTSEnabled(){
    int status;
    GetCommModemStatus(Cport, (LPDWORD)((void *)&status));
    return (status & MS_CTS_ON)? 1 : 0;
}
