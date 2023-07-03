#ifndef SAKURAJIN_RS232_NATIVE_HPP_INCLUDED
#define SAKURAJIN_RS232_NATIVE_HPP_INCLUDED

#ifndef RS232_EXPORT_MACRO
    #define RS232_EXPORT_MACRO
#endif

#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__)
    #define RS232_UNIX
#endif

#include "rs232_baudrate_impl.hpp"

#include <atomic>
#include <iostream>
#include <memory>
#include <ostream>
#include <shared_mutex>
#include <string>
#include <string_view>

namespace sakurajin {
    /**
     * @brief An enum to indicate the status of the connection
     *
     */
    enum connectionStatus {
        /// The connection is established and the port is available
        connected,
        /// The connection is not established but the port is available
        disconnected,
        /// The port is not found (device not connected or wrong name)
        portNotFound,
        /// Communication with the port is not possible
        otherError
    };


    /**
     * @brief An enum to define all the port status flags
     * The assigned names are chosen in a way to make the code platform agnostic.
     * On Unix the values are the same as the ones defined in ioctl.h.
     * On Windows the values are the same as the ones defined in winbase.h.
     */
    enum portStatusFlags {
#ifdef RS232_UNIX
        DATA_SET_READY_LINE_ENABLE = TIOCM_LE,
        DATA_TERMINAL_READY        = TIOCM_DTR,
        REQUEST_TO_SEND            = TIOCM_RTS,
        SECONDARY_TXD              = TIOCM_ST,
        SECONDARY_RXD              = TIOCM_SR,
        CLEAR_TO_SEND              = TIOCM_CTS,
        DATA_CARRIER_DETECT        = TIOCM_CAR,
        RING                       = TIOCM_RNG,
        DATA_SET_READY_2           = TIOCM_DSR,
#endif
    };

    /**
     * @brief The native implementation of RS232
     *
     * The native implementation of RS232 is a class that is only used internally.
     * It is used to implement the basic raw read and write functions for each platform.
     * Because it is platform specific, the source file is split into multiple files.
     *
     * The whole class is supposed to be thread safe.
     * If something is not then that is treated as a bug.
     */
    class RS232_EXPORT_MACRO RS232_native {
      private:
        /**
         * @brief The name of the connected port.
         * This variable is const to prevent accidental changes during the lifetime of the object
         *
         */
        const std::string devname;

        /**
         * @brief A boolean that indicates if the port is available and a connection has been established
         *
         */
        std::atomic<connectionStatus> connStatus = connectionStatus::disconnected;

        /**
         * @brief The platform specific handle to the port
         *
         * This is a void pointer to prevent the need of including the platform specific header files.
         * On windows this points to a HANDLE and on unix to an int.
         */
        std::atomic<void*> portHandle = nullptr;

        /**
         * @brief The platform specific configuration of the port
         *
         * This is a void pointer to prevent the need of including the platform specific header files.
         * On windows this points to a DCB and on unix to a termios.
         */
        std::atomic<void*> portConfig = nullptr;

        /**
         * @brief The mutex that is used to prevent multiple threads from accessing the port at the same time
         *
         */
        std::shared_mutex dataAccessMutex;

      public:
        /**
         * @brief Construct a new RS232 object
         *
         * @param deviceName The name of the port where the device is connected to
         */
        RS232_native(const std::string& deviceName, Baudrate Rate, std::ostream& error_stream = std::cerr);

        /**
         * @brief Destroy the RS232 object
         *
         */
        ~RS232_native();

        /**
         * @brief Establish a connection to the port
         * @note this is always called by the constructor so by default there should be a connection
         * @return connectionStatus the status of the connection
         */
        connectionStatus connect(Baudrate Rate, std::ostream& error_stream = std::cerr);

        /**
         * @brief disconnects the port and prevents further access
         * This function will always close the connection and will not throw an exception
         */
        void disconnect() noexcept;

        /*
         * @brief The platform specific function to read a string from the port
         * @param data the data that should be read from the port
         * @param length the length of the data that should be read from the port
         * @return int the number of bytes that were read from the port
         */
        [[nodiscard]]
        int readRawData(unsigned char* data_location, int length);

        /*
         * @brief The platform specific function to write a string to the port
         * @param data the data that should be written to the port
         * @param length the length of the data that should be written to the port
         * @return int the number of bytes that were written to the port
         */
        [[nodiscard]]
        int writeRawData(unsigned char* data_location, int length);

        /**
         * @brief Checks if the connection was started successfully
         *
         * @return true the connection is established as expected
         * @return false there was an error while initializing the connection or some of the settings are not valid
         */
        [[nodiscard]]
        connectionStatus getConnectionStatus();

        /**
         * @brief Get the name of the port used for this RS232 connection
         */
        [[nodiscard]]
        std::string_view getDeviceName() const;

        /**
         * @brief check if a given connection flag is set
         * @param flag The flag that should be checked
         * @return true the flag is set
         * @return false the flag is not set or no connection is established
         */
        [[nodiscard]]
        bool checkForFlag(portStatusFlags flag);
    };

} // namespace sakurajin

#endif // SAKURAJIN_RS232_NATIVE_HPP_INCLUDED
