/*
***********************************************
*
* Author: Frank Andre Moreno Vera
*
* Copyright (C) 2014 Frank Andre Moreno Vera
*
* frank@ariot.pe
*
***********************************************
*/

#ifndef github_com_kranfix_rs232_rs232_h
#define github_com_kranfix_rs232_rs232_h

#include <stdio.h>
#include <string.h>

#ifdef __linux__

#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#else

#include <windows.h>

#endif

//#undef __cplusplus
#ifdef __cplusplus
namespace kfx {

#  ifdef __linux__
    const char Comports[22][13] = {"/dev/ttyACM0",
        "/dev/ttyS1", "/dev/ttyS2", "/dev/ttyS3",
        "/dev/ttyS4", "/dev/ttyS5", "/dev/ttyS6",
        "/dev/ttyS7", "/dev/ttyS8", "/dev/ttyS9",
        "/dev/ttyS10", "/dev/ttyS11", "/dev/ttyS12",
        "/dev/ttyS13", "/dev/ttyS14", "/dev/ttyS15",
        "/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2",
        "/dev/ttyUSB3", "/dev/ttyUSB4", "/dev/ttyUSB5"};
#  else
    const char Comports[16][10]={"\\\\.\\COM1",
        "\\\\.\\COM2", "\\\\.\\COM3",  "\\\\.\\COM4",
        "\\\\.\\COM5",  "\\\\.\\COM6", "\\\\.\\COM7",
        "\\\\.\\COM8", "\\\\.\\COM9",  "\\\\.\\COM10",
        "\\\\.\\COM11", "\\\\.\\COM12", "\\\\.\\COM13",
        "\\\\.\\COM14", "\\\\.\\COM15", "\\\\.\\COM16"};
#  endif

class RS232
{
    char devname[13];   // Device Name
    int baudr, port;    // Baudrate and Port Number
    bool available;
    struct termios ops; // old port settings
public:
    RS232(char *, int);
    int IsAvailable() { return available; }
    char * GetDeviceName() { return devname; }
    int Read(unsigned char);
    int Read(unsigned char *, int);
    int Write(unsigned char);
    int Write(unsigned char *, int);
    void Print(const char *);
    void Close();
    int IsCTSEnabled();
};

}

#endif



#endif // giihub_com_kranfix_rs232_rs232_h
