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

#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))

    #include <termios.h>

#else
    #include <windows.h>
#endif

namespace sakurajin {

    #if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
        enum Baudrate{
            baud50 = B50,
            baud75 = B75,
            baud110 = B110,
            baud134 = B134,
            baud150 = B150,
            baud200 = B200,
            baud300 = B300,
            baud600 = B600,
            baud1200 = B1200,
            baud1800 = B1800,
            baud2400 = B2400,
            baud4800 = B4800,
            baud9600 = B9600,
            baud19200 = B19200,
            baud38400 = B38400,
            baud57600 = B57600,
            baud115200 = B115200,
            baud230400 = B230400,
            baud460800 = B460800,
            baud500000 = B500000,
            baud576000 = B576000,
            baud921600 = B921600,
            baud1000000 = B1000000
        };
    #else
        enum Baudrate{
            baud110 = 110,
            baud300 = 300,
            baud600 = 600,
            baud1200 = 1200,
            baud2400 = 2400,
            baud4800 = 4800,
            baud9600 = 9600,
            baud19200 = 19200,
            baud38400 = 38400,
            baud57600 = 57600,
            baud115200 = 115200,
            baud128000 = 128000,
            baud256000 = 256000
        };
    #endif

    class RS232{
    private:
        /**
         * @brief The name of the connected port.
         * This vairable is const to prevent accidental changes during the lifetime of the object
         * 
         */
        const std::string devname;

        int r, port;    // Baudrate and Port Number
        bool available;

        #if defined(unix) || defined(__unix__) || defined(__unix)
            struct termios ops; // old port settings
        #else
            HANDLE Cport;
        #endif

        
        int Read(unsigned char *, int);
        int Write(unsigned char *, int);
        
    public:
        /**
         * @brief Construct a new RS232 object
         * 
         * @param deviceName The name of the port where the device is connected to
         * @param Rate The  rate that should be used for the connection
         */
        RS232(const std::string& deviceName, Baudrate Rate);

        /**
         * @brief Destroy the RS232.
         * This function calls the Close() function automatically to leave the port in a valid state.
         * 
         */
        ~RS232();

        /**
         * @brief Checks if the connection was started sucessfully
         * 
         * @return true the connection is established as expected
         * @return false there was an error while initializing the connection or some of the settings are not valid
         */
        bool IsAvailable() const;

        /**
         * @brief Get the name of the port used for this RS232 connection
         */
        std::string_view GetDeviceName() const;
        
        /**
         * @brief reads until the next character is received
         * 
         * @return std::tuple<unsigned char, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        std::tuple<unsigned char, int> ReadNextChar();

        /**
         * @brief reads the interface until a newline (\n) is received
         * 
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        std::tuple<std::string, int> ReadNextMessage();

        /**
         * @brief read the interface until one of the stop conditions is reached
         * 
         * @param conditions a vector containing all the stop conditions.
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        std::tuple<std::string, int> ReadUntil(std::vector<unsigned char> conditions);

        void Print(const std::string& text);

        /**
         * @brief close the connection to the device.
         * @warning this disables all following transactions to the device.
         * 
         */
        void Close();
        int IsCTSEnabled();
    };

}
