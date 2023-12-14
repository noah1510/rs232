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
#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <ostream>
#include <regex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>

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

    RS232_EXPORT_MACRO std::vector<std::string> getAvailablePorts() noexcept;
    RS232_EXPORT_MACRO std::vector<std::string> getMatchingPorts(const std::regex& pattern) noexcept;

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
         * @brief The baudrate this device is connected with.
         * This is used in the reconnect method to reestablish the connection with the same baudrate as before.
         */
        std::atomic<Baudrate> baudrate = Baudrate::baud9600;

        /**
         * @brief The mutex that is used to prevent multiple threads from accessing the port at the same time
         *
         */
        std::shared_mutex dataAccessMutex;

        template <typename retVal>
        using encapsulatedFunction = std::function<retVal()>;

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
        template <typename retT>
        int64_t callWithOptionalLock(const encapsulatedFunction<retT>& func, bool block = true) {
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
            return static_cast<int64_t>(retVal);
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
         * this is always called by the constructor so by default there should be a connection
         * @note This operation can take some time since the mutex has to be locked.
         * @note if the connection is already established this will exit early. To force a reconnect use disconnect() first.
         *
         * @param error_stream the stream where the error messages should be written to
         * @return connectionStatus the status of the connection
         */
        connectionStatus connect(std::ostream& error_stream = std::cerr) noexcept;

        /**
         * @brief Change the baudrate of the connection
         * This function disconnects the port and reconnects it with the new baudrate.
         * @param Rate the new baudrate
         * @param error_stream the stream where the error messages should be written to
         * @return true the baudrate was changed and a connection was established
         */
        bool changeBaudrate(Baudrate Rate, std::ostream& error_stream = std::cerr) noexcept;

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
        int64_t readRawData(char* data_location, int length, bool block = true) noexcept;

        /**
         * @brief The platform specific function to write a string to the port
         * @param data the data that should be written to the port
         * @param length the length of the data that should be written to the port
         * @param block set to true if you want this code to block. If set to false this returns early if the mutex cannot be locked
         * @return int the number of bytes that were written to the port
         */
        [[nodiscard]]
        int64_t writeRawData(char* data_location, int length, bool block = true) noexcept;

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
        int64_t retrieveFlags(bool block = true) noexcept;
    };

    /**
     * @brief This namespace contains some external functions that operate on the RS232_native class
     * They are not directly part of the class since they are abstractions in a way but they are still useful.
     * Use these functions for convenient direct access to the RS232_native class.
     *
     * In version 1.x these were part of the RS232 wrapper class but they were moved here.
     * This is because the wrapper class uses a new design where these functions no longer fit.
     */
    namespace native {

        /**
         * @brief reads until the next character is received or the waitTaime is over
         *
         * @param waitTime the duration that should be waited for a signal before stopping the function.
         * @param ignoreTime true if the duration value should be ignored (the same as no parameter)
         *
         * @return std::tuple<unsigned char, int> this tuple contains the wanted return data and an error code in case something went wrong
         * The return value is >= 0 if everything is okay and < 0 if something went wrong
         */
        template <class Rep = int64_t, class Period = std::ratio<1>>
        [[nodiscard]] [[maybe_unused]]
        std::tuple<unsigned char, int> ReadNextChar(const std::shared_ptr<RS232_native>& transferDevice,
                                                    std::chrono::duration<Rep, Period>   waitTime,
                                                    bool                                 ignoreTime = false) {
            if (transferDevice == nullptr) {
                return {'\0', -1};
            }

            if (transferDevice->getConnectionStatus() != sakurajin::connectionStatus::connected) {
                return {'\0', -2};
            }

            auto startTime = std::chrono::high_resolution_clock::now();

            char    IOBuf = '\0';
            int64_t readLength;

            do {
                readLength = transferDevice->readRawData(&IOBuf, 1, false);

                if (ignoreTime) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                    continue;
                }

                if (std::chrono::high_resolution_clock::now() - startTime > waitTime) {
                    return {'\0', -3};
                }
            } while (readLength < 1);

            return {IOBuf, 0};
        }

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
        template <class Rep = int64_t, class Period = std::ratio<1>>
        [[nodiscard]] [[maybe_unused]]
        std::tuple<std::string, int> ReadUntil(const std::shared_ptr<RS232_native>& transferDevice,
                                               const std::vector<unsigned char>&    conditions,
                                               std::chrono::duration<Rep, Period>   waitTime,
                                               bool                                 ignoreTime = false) {
            if (transferDevice == nullptr) {
                return {"", -1};
            }

            if (transferDevice->getConnectionStatus() != sakurajin::connectionStatus::connected) {
                return {"", -2};
            }

            std::string message;
            int64_t     errCode  = 0;
            char        nextChar = '\n';
            bool        stop     = false;

            while (!stop) {
                // read the next char and append if there was no error
                std::tie(nextChar, errCode) = ReadNextChar(transferDevice, waitTime, ignoreTime);

                if (errCode < 0) {
                    return {"", -3};
                }
                message += nextChar;

                // check if a stop condition is met
                for (auto cond : conditions) {
                    if (cond == nextChar) {
                        stop = true;
                        break;
                    }
                }
            }

            return {message, 0};
        }

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
        template <class Rep = int64_t, class Period = std::ratio<1>>
        [[nodiscard]] [[maybe_unused]]
        std::tuple<std::string, int> ReadNextMessage(const std::shared_ptr<RS232_native>& transferDevice,
                                                     std::chrono::duration<Rep, Period>   waitTime,
                                                     bool                                 ignoreTime = false) {
            return ReadUntil(transferDevice, {'\n'}, waitTime, ignoreTime);
        }

        /**
         * @brief Directly output a string to the device.
         * The string is written to the device and the function returns immediately after that.
         * @note this function is blocking and will wait until a write can actually be performed.
         * @param transferDevice The device that should be used for the transfer
         * @param text The text that should be written to the device
         * @return negative if an error occurred
         */
        [[maybe_unused]]
        RS232_EXPORT_MACRO int Print(const std::shared_ptr<RS232_native>& transferDevice, const std::string& text);

        /**
         * @brief reads until the next character is received
         *
         * @return std::tuple<unsigned char, int> this tuple contains the wanted return data and an error code in case something went wrong
         * The return value is >= 0 if everything is okay and < 0 if something went wrong
         */
        [[nodiscard]] [[maybe_unused]]
        RS232_EXPORT_MACRO std::tuple<unsigned char, int> ReadNextChar(const std::shared_ptr<RS232_native>& transferDevice);

        /**
         * @brief reads the interface until a newline (\n) is received
         *
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        [[nodiscard]] [[maybe_unused]]
        RS232_EXPORT_MACRO std::tuple<std::string, int> ReadNextMessage(const std::shared_ptr<RS232_native>& transferDevice);

        /**
         * @brief read the interface until one of the stop conditions is reached
         *
         * @param conditions a vector containing all the stop conditions.
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        [[nodiscard]] [[maybe_unused]]
        RS232_EXPORT_MACRO std::tuple<std::string, int> ReadUntil(const std::shared_ptr<RS232_native>& transferDevice,
                                                                  const std::vector<unsigned char>&    conditions);
    } // namespace native

} // namespace sakurajin

#endif // SAKURAJIN_RS232_NATIVE_HPP_INCLUDED
