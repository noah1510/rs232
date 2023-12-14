#pragma once

#ifndef SAKURAJIN_RS232_NATIVE_HPP_INCLUDED
    #error "do not directly include this header!"
#endif

#ifdef RS232_UNIX
    #include <sys/ioctl.h>
    #include <termios.h>
#else
    #ifndef B110
        #define B110 110
    #endif
    #ifndef B300
        #define B300 300
    #endif
    #ifndef B600
        #define B600 600
    #endif
    #ifndef B1200
        #define B1200 1200
    #endif
    #ifndef B2400
        #define B2400 2400
    #endif
    #ifndef B4800
        #define B4800 4800
    #endif
    #ifndef B9600
        #define B9600 9600
    #endif
    #ifndef B19200
        #define B19200 19200
    #endif
    #ifndef B38400
        #define B38400 38400
    #endif
    #ifndef B57600
        #define B57600 57600
    #endif
    #ifndef B115200
        #define B115200 115200
    #endif

    // Windows specific baudrates
    #ifndef B14400
        #define B14400 14400
    #endif
    #ifndef B128000
        #define B128000 128000
    #endif
    #ifndef B256000
        #define B256000 256000
    #endif
#endif

namespace sakurajin {
    enum Baudrate {
        baud110   = B110,
        baud300   = B300,
        baud600   = B600,
        baud1200  = B1200,
        baud2400  = B2400,
        baud4800  = B4800,
        baud9600  = B9600,
        baud19200 = B19200,
        baud38400 = B38400,
#ifdef B57600
        baud57600 = B57600,
#endif
#ifdef B115200
        baud115200 = B115200,
#endif
// if enabled add the platform specific baudrates
// these are not defined on all platforms and might even differ from compiler to compiler
// so they are disabled by default and can be enabled by defining RS232_PLATFORM_SPECIFC_RATES before including the header
#ifdef RS232_PLATFORM_SPECIFC_RATES
    #ifdef B50
        baud50 = B50,
    #endif
    #ifdef B75
        baud75 = B75,
    #endif
    #ifdef B134
        baud134 = B134,
    #endif
    #ifdef B150
        baud150 = B150,
    #endif
    #ifdef B200
        baud200 = B200,
    #endif
    #ifdef B1800
        baud1800 = B1800,
    #endif
    #ifdef B14400
        baud14400 = B14400,
    #endif
    #ifdef B28800
        baud28800 = B28800,
    #endif

    #ifdef B128000
        baud128000 = B128000,
    #endif
    #ifdef B230400
        baud230400 = B230400,
    #endif
    #ifdef B256000
        baud256000 = B256000,
    #endif
    #ifdef B460800
        baud460800 = B460800,
    #endif
    #ifdef B500000
        baud500000 = B500000,
    #endif
    #ifdef B576000
        baud576000 = B576000,
    #endif
    #ifdef B921600
        baud921600 = B921600,
    #endif
    #ifdef B1000000
        baud1000000 = B1000000,
    #endif
    #ifdef B1152000
        baud1152000 = B1152000,
    #endif
    #ifdef B1500000
        baud1500000 = B1500000,
    #endif
    #ifdef B2000000
        baud2000000 = B2000000,
    #endif
    #ifdef B2500000
        baud2500000 = B2500000,
    #endif
    #ifdef B3000000
        baud3000000 = B3000000,
    #endif
    #ifdef B3500000
        baud3500000 = B3500000,
    #endif
    #ifdef B4000000
        baud4000000 = B4000000,
    #endif
#endif
    };
} // namespace sakurajin
