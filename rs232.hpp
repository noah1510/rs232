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

namespace kfx {

    #  ifdef __linux__
        const std::string Comports[] = {"/dev/ttyACM0",
            "/dev/ttyS1", "/dev/ttyS2", "/dev/ttyS3",
            "/dev/ttyS4", "/dev/ttyS5", "/dev/ttyS6",
            "/dev/ttyS7", "/dev/ttyS8", "/dev/ttyS9",
            "/dev/ttyS10", "/dev/ttyS11", "/dev/ttyS12",
            "/dev/ttyS13", "/dev/ttyS14", "/dev/ttyS15",
            "/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2",
            "/dev/ttyUSB3", "/dev/ttyUSB4", "/dev/ttyUSB5"};
    #  else
        const std::string Comports[] = {"\\\\.\\COM1",
            "\\\\.\\COM2", "\\\\.\\COM3",  "\\\\.\\COM4",
            "\\\\.\\COM5",  "\\\\.\\COM6", "\\\\.\\COM7",
            "\\\\.\\COM8", "\\\\.\\COM9",  "\\\\.\\COM10",
            "\\\\.\\COM11", "\\\\.\\COM12", "\\\\.\\COM13",
            "\\\\.\\COM14", "\\\\.\\COM15", "\\\\.\\COM16"};
    #  endif

    class RS232{
    private:
        /**
         * @brief The name of the connected port.
         * This vairable is const to prevent accidental changes during the lifetime of the object
         * 
         */
        const std::string devname;

        int baudr, port;    // Baudrate and Port Number
        bool available;
        struct termios ops; // old port settings
    public:
        /**
         * @brief Construct a new RS232 object
         * 
         * @param deviceName The name of the port where the device is connected to
         * @param baudRate The baud rate that should be used for the connection
         */
        RS232(const std::string& deviceName, int baudRate);

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

        int Read(unsigned char);
        int Read(unsigned char *, int);
        int Write(unsigned char);
        int Write(unsigned char *, int);
        /**
         * @brief close the connection to the device.
         * @warning this disables all following transactions to the device.
         * 
         */
        void Close();
        int IsCTSEnabled();
    };

}
