/*
***********************************************
*
* Author: Frank Andre Moreno Vera
*
* Copyright (C) 2014 Frank Andre Moreno Vera
*
* frankmoreno1993@gmail.com
*
***********************************************
*/

#include "rs232.hpp"

#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

sakurajin::RS232::RS232(const std::string& deviceName, Baudrate baudrate) : devname(deviceName), available(false){

    // Chossing baudrate
    unsigned int baudr = baudrate;
    
    port = open(devname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if(port == -1){
        std::cerr << "unable to open comport "<< std::endl  ;
        return;
    }

    portHandle = static_cast<void*>(new termios{});
    int error = tcgetattr(port, static_cast<struct termios*>(portHandle));
    if(error == -1){
        close(port);
        std::cerr << "unable to read portsettings " << std::endl;
        return;
    }
    
    struct termios nps{};

    nps.c_cflag = baudr | CS8 | CLOCAL | CREAD;
    nps.c_iflag = IGNPAR;
    nps.c_oflag = 0;
    nps.c_lflag = 0;
    nps.c_cc[VMIN] = 0;      /* block untill n bytes are received */
    nps.c_cc[VTIME] = 0;     /* block untill a timer expires (n * 100 mSec.) */
    
    error = tcsetattr(port, TCSANOW, &nps);
    if(error == -1){
        close(port);
        std::cerr << "unable to adjust portsettings " << std::endl;
        return;
    }

    available = true;
}

int sakurajin::RS232::Read(unsigned char *buf, int size){
    if (!available){
        return -1;
    }

    int limit = 
    #ifndef __STRICT_ANSI__                       /* __STRICT_ANSI__ is defined when the -ansi option is used for gcc */
        (int)SSIZE_MAX;  /* SSIZE_MAX is defined in limits.h */
    #else
        4096;
    #endif
        
    size = std::clamp(size, 0, limit);

    return read(port, buf, size);
}

int sakurajin::RS232::Write(unsigned char *buf, int size){
    if (!available){
        return -1;
    }

    return write(port, buf, size);
}

void sakurajin::RS232::Close(){
    available = false;
    close(port);
    tcsetattr(port, TCSANOW, static_cast<struct termios*>(portHandle));
    delete static_cast<struct termios*>(portHandle);
}

/*
Constant    Description
--------------------------------------------
TIOCM_LE    DSR (data set ready/line enable)
TIOCM_DTR   DTR (data terminal ready)
TIOCM_RTS   RTS (request to send)
TIOCM_ST    Secondary TXD (transmit)
TIOCM_SR    Secondary RXD (receive)
TIOCM_CTS   CTS (clear to send)
TIOCM_CAR   DCD (data carrier detect)
TIOCM_CD    Synonym for TIOCM_CAR
TIOCM_RNG   RNG (ring)
TIOCM_RI    Synonym for TIOCM_RNG
TIOCM_DSR   DSR (data set ready)
*/
bool sakurajin::RS232::IsCTSEnabled(){
    int status;
    status = ioctl(port, TIOCMGET, &status);
    return status & TIOCM_CTS;
}
