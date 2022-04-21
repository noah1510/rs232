﻿/*
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

#include "stdio.h"
#include "string.h"


#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

kfx::RS232::RS232(const std::string& deviceName, Baudrate baudrate) : devname(deviceName), available(false){

    // Chossing baudrate
    unsigned int baudr = baudrate;
    
    port = open(devname.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if(port == -1){
        perror("unable to open comport ");
        return;
    }

    int error = tcgetattr(port, &ops);
    if(error == -1){
        close(port);
        perror("unable to read portsettings ");
        return;
    }
    
    struct termios nps;

    memset(&nps, 0, sizeof(nps));  /* clear the new struct */

    nps.c_cflag = baudr | CS8 | CLOCAL | CREAD;
    nps.c_iflag = IGNPAR;
    nps.c_oflag = 0;
    nps.c_lflag = 0;
    nps.c_cc[VMIN] = 0;      /* block untill n bytes are received */
    nps.c_cc[VTIME] = 0;     /* block untill a timer expires (n * 100 mSec.) */
    
    error = tcsetattr(port, TCSANOW, &nps);
    if(error == -1){
        close(port);
        perror("unable to adjust portsettings ");
        return;
    }

    available = true;
}

int kfx::RS232::Read(unsigned char byte){
    if (!available){
        return -1;
    }

    return read(port, &byte, 1);
}

int kfx::RS232::Read(unsigned char *buf, int size){
    if (!available){
        return -1;
    }

    #ifndef __STRICT_ANSI__                       /* __STRICT_ANSI__ is defined when the -ansi option is used for gcc */
        if(size > SSIZE_MAX)  size = (int)SSIZE_MAX;  /* SSIZE_MAX is defined in limits.h */
    #else
        if(size>4096)  size = 4096;
    #endif

    return read(port, buf, size);
}

int kfx::RS232::Write(unsigned char byte){
    if (!available){
        return -1;
    }

    return write(port, &byte, 1);
}

int kfx::RS232::Write(unsigned char *buf, int size){
    if (!available){
        return -1;
    }

    return write(port, buf, size);
}

void kfx::RS232::Close(){
    available = false;
    close(port);
    tcsetattr(port, TCSANOW, &ops);
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
int kfx::RS232::IsCTSEnabled(){
    int status;
    status = ioctl(port, TIOCMGET, &status);
    return (status & TIOCM_CTS)? 1 : 0;
}
