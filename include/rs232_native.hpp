#ifndef SAKURAJIN_RS232_NATIVE_HPP_INCLUDED
#define SAKURAJIN_RS232_NATIVE_HPP_INCLUDED

#ifndef RS232_EXPORT_MACRO
    #define RS232_EXPORT_MACRO
#endif

#if defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__)
    #define RS232_UNIX
#endif

#include "rs232_baudrate_impl.hpp"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <regex>
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
        CLEAR_TO_SEND              = TIOCM_CTS,
        DATA_CARRIER_DETECT        = TIOCM_CAR,
        RING                       = TIOCM_RNG,
        DATA_SET_READY_2           = TIOCM_DSR
#else
        DATA_SET_READY_LINE_ENABLE = 0x0020, // same as DATA_SET_READY_2
        DATA_TERMINAL_READY        = 0x0080, // same as DATA_CARIEER_DETECT
        REQUEST_TO_SEND            = 0x0010, // same as CLEAR_TO_SEND
        CLEAR_TO_SEND              = 0x0010, // the value of MS_CTS_ON
        DATA_CARRIER_DETECT        = 0x0080, // the value of MS_RLSD_ON
        RING                       = 0x0040, // the value of MS_RING_ON
        DATA_SET_READY_2           = 0x0020  // the value of MS_DSR_ON
#endif
    };

    RS232_EXPORT_MACRO std::vector<std::string> getAvailablePorts();
    RS232_EXPORT_MACRO std::vector<std::string> getMatchingPorts(const std::regex& pattern);

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

        /**
         * @brief A method to make a function call with a lock on the mutex
         * This method is used to prevent code duplication.
         * In case the block parameter is true, the mutex will be locked before the function call and unlocked afterwards.
         * If the block parameter is false, the mutex will be tried to lock and if that is not immediately possible -1 will be returned.
         * In that case the passed function will not be called.
         *
         * @tparam args The types of the arguments that should be passed to the function
         * @param lock A boolean that indicates if the mutex should be waited for or not
         * @param func The function that should be called
         * @return ssize_t The return value of the function or -1 if something went wrong
         */
        ssize_t callWithOptionalLock(bool block, const std::function<ssize_t()>& func) {
            // lock the mutex if the lock parameter is true
            // otherwise exit if the lock cannot be acquired
            if (block) {
                dataAccessMutex.lock();
            } else if (!dataAccessMutex.try_lock()) {
                return -1;
            }

            // call the function and unlock the mutex before passing the return value.
            auto retVal = func();
            dataAccessMutex.unlock();
            return retVal;
        }

      public:
        /**
         * @brief Construct a new RS232 object
         *
         * @param deviceName The name of the port where the device is connected to
         */
        RS232_native(std::string deviceName, Baudrate Rate, std::ostream& error_stream = std::cerr);

        /**
         * @brief Destroy the RS232 object
         *
         */
        ~RS232_native();

        /**
         * @brief Establish a connection to the port
         * @note this is always called by the constructor so by default there should be a connection
         * @note This operation can take some time since the mutex has to be locked.
         * @return connectionStatus the status of the connection
         */
        connectionStatus connect(Baudrate Rate, std::ostream& error_stream = std::cerr) noexcept;

        /**
         * @brief disconnects the port and prevents further access
         * This function will always close the connection and will not throw an exception
         * @note This operation can take some time since the mutex has to be locked.
         */
        void disconnect() noexcept;

        /**
         * @brief The platform specific function to read a string from the port
         * @param data the data that should be read from the port
         * @param length the length of the data that should be read from the port
         * @param block set to true if you want this code to block. If set to false this returns early if the mutex cannot be locked
         * @return int the number of bytes that were read from the port
         */
        [[nodiscard]]
        ssize_t readRawData(char* data_location, int length, bool block = true) noexcept;

        /**
         * @brief The platform specific function to write a string to the port
         * @param data the data that should be written to the port
         * @param length the length of the data that should be written to the port
         * @param block set to true if you want this code to block. If set to false this returns early if the mutex cannot be locked
         * @return int the number of bytes that were written to the port
         */
        [[nodiscard]]
        ssize_t writeRawData(char* data_location, int length, bool block = true) noexcept;

        /**
         * @brief Checks if the connection was started successfully
         *
         * @return true the connection is established as expected
         * @return false there was an error while initializing the connection or some of the settings are not valid
         */
        [[nodiscard]]
        connectionStatus getConnectionStatus() noexcept;

        /**
         * @brief Get the name of the port used for this RS232 connection
         */
        [[nodiscard]]
        std::string_view getDeviceName() const noexcept;

        /**
         * @brief check if a given connection flag is set
         * @param flag The flag that should be checked
         * @note This operation can take some time since the mutex has to be locked.
         * @return true the flag is set
         * @return false the flag is not set or no connection is established
         */
        [[nodiscard]]
        bool checkForFlag(portStatusFlags flag, bool block = true) noexcept;

        /**
         * @brief retrieve all the flags that are set
         * @note This operation can take some time since the mutex has to be locked.
         * @return int the flags that are set
         */
        [[nodiscard]]
        ssize_t retrieveFlags(bool block = true) noexcept;
    };

} // namespace sakurajin

#endif // SAKURAJIN_RS232_NATIVE_HPP_INCLUDED
