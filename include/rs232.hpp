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
#endif

#include "rs232_baudrate_impl.hpp"

#ifndef RS232_EXPORT_MACRO
    #define RS232_EXPORT_MACRO
#endif

namespace sakurajin {

    class RS232_EXPORT_MACRO RS232{
    private:
        /**
         * @brief The name of the connected port.
         * This vairable is const to prevent accidental changes during the lifetime of the object
         * 
         */
        const std::string devname;

        /**
         * @brief A boolean that indicates if the port is available and a connection has been established
         *
         */
        bool available;

        /**
         * @brief The platform specific handle to the port
         *
         * This is a void pointer to prevent the need of including the platform specific header files.
         * On windows this points to a HANDLE and on unix to an int.
         */
        void* portHandle = nullptr;

        /**
         * @brief The platform specific configuration of the port
         *
         * This is a void pointer to prevent the need of including the platform specific header files.
         * On windows this points to a DCB and on unix to a termios.
         */
        void* portConfig = nullptr;

        /*
         * @brief The platform specific function to read a string from the port
         * @param data the data that should be read from the port
         * @param length the length of the data that should be read from the port
         * @return int the number of bytes that were read from the port
         */
        int Read(unsigned char *, int);

        /*
         * @brief The platform specific function to write a string to the port
         * @param data the data that should be written to the port
         * @param length the length of the data that should be written to the port
         * @return int the number of bytes that were written to the port
         */
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
        [[nodiscard]]
        bool IsAvailable() const;

        /**
         * @brief Get the name of the port used for this RS232 connection
         */
        [[nodiscard]]
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
        [[nodiscard]]
        bool IsCTSEnabled();
    };

}

#include "rs232_template_implementations.hpp"

#endif // RS232_HPP_INCLUDED
