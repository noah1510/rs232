#pragma once

#ifdef SAKURAJIN_RS232_HPP_INCLUDED
    #ifndef RS232_UNIX
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
    #endif

    namespace sakurajin{
        enum Baudrate{
            baud110 = B110,
            baud300 = B300,
            baud600 = B600,
            baud1200 = B1200,
            baud2400 = B2400,
            baud4800 = B4800,
            baud9600 = B9600,
            baud19200 = B19200,
            baud38400 = B38400,
            baud57600 = B57600,
            baud115200 = B115200,
        };
    }
#else

    #error "do not directly include this header!"

#endif
