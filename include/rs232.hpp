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



#ifndef SAKURAJIN_RS232_HPP_INCLUDED
#define SAKURAJIN_RS232_HPP_INCLUDED

#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <chrono>

#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__)
    #define RS232_UNIX

    #include <termios.h>

#else
    #include <WinDef.h>
#endif

#ifndef RS232_EXPORT_MACRO
    #define RS232_EXPORT_MACRO
#endif

namespace sakurajin {

    #ifdef RS232_UNIX
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
            baud230400 = B230400
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

    class RS232_EXPORT_MACRO RS232{
    private:
        /**
         * @brief The name of the connected port.
         * This vairable is const to prevent accidental changes during the lifetime of the object
         * 
         */
        const std::string devname;

        int r, port;    // Baudrate and Port Number
        bool available;

        #ifdef RS232_UNIX
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
         * The return value is >= 0 if everything is okay and < 0 if something went wrong
         */
        std::tuple<unsigned char, int> ReadNextChar();
        
        /**
         * @brief reads until the next character is received or the waitTaime is over
         * 
         * @param waitTime the duration that should be waited for a signal before stopping the function.
         * @param ignoreTime true if the duration value should be ignored (the same as no parameter)
         * 
         * @return std::tuple<unsigned char, int> this tuple contains the wanted return data and an error code in case something went wrong
         * The return value is >= 0 if everything is okay and < 0 if something went wrong
         */
        template<class Rep = int64_t, class Period = std::ratio<1> >
        std::tuple<unsigned char, int> ReadNextChar(std::chrono::duration<Rep, Period> waitTime, bool ignoreTime = false);

        /**
         * @brief reads the interface until a newline (\n) is received
         * 
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        std::tuple<std::string, int> ReadNextMessage();
        
        /**
         * @brief reads the interface until a newline (\n) is received or the waitTime is over
         * The waitTime is the time the code will wait for each next character. If the delay between the 
         * characters is too long the function will return an error.
         * 
         * @param waitTime the duration that should be waited for a signal before stopping the function.
         * @param ignoreTime true if the duration value should be ignored (the same as no parameter)
         * 
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        template<class Rep = int64_t, class Period = std::ratio<1> >
        std::tuple<std::string, int> ReadNextMessage(
            std::chrono::duration<Rep, Period> waitTime, 
            bool ignoreTime = false
        );

        /**
         * @brief read the interface until one of the stop conditions is reached
         * 
         * @param conditions a vector containing all the stop conditions.
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        std::tuple<std::string, int> ReadUntil(const std::vector<unsigned char>& conditions);

        /**
         * @brief read the interface until one of the stop conditions is reached or the waitTaime is over
         * The waitTime is the time the code will wait for each next character. If the delay between the 
         * characters is too long the function will return an error.
         * 
         * @param waitTime the duration that should be waited for a signal before stopping the function.
         * @param ignoreTime true if the duration value should be ignored (the same as no parameter)
         * 
         * @param conditions a vector containing all the stop conditions.
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        template<class Rep = int64_t, class Period = std::ratio<1> >
        std::tuple<std::string, int> ReadUntil(
            const std::vector<unsigned char>& conditions, 
            std::chrono::duration<Rep, Period> waitTime, 
            bool ignoreTime = false
        );

        void Print(const std::string& text);

        /**
         * @brief close the connection to the device.
         * @warning this disables all following transactions to the device.
         * 
         */
        void Close();
        
        /**
         * @brief check if the clear to send flag is set
         * 
         * @return true if the flag is set
         * @return false if the flag is not set
         */
        bool IsCTSEnabled();
    };

}

#include "rs232_template_implementations.hpp"

#endif // RS232_HPP_INCLUDED
