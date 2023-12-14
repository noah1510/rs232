#ifndef SAKURAJIN_RS232_HPP_INCLUDED
#define SAKURAJIN_RS232_HPP_INCLUDED

#include "rs232_native.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace sakurajin {

    class RS232_EXPORT_MACRO RS232 {
      private:
        /**
         * @brief The list of devices that are used for the connection
         * This wrapper class uses a list of devices to allow the user to switch between devices without having to close the connection.
         * This is useful when the user wants to use the same port for different purposes.
         * Another purpose is to allow the user to specify different port names for the same device.
         * This is needed since the port name can change depending on the operating system.
         * For details on what capabilities are supported by the different operating systems see the documentation of the RS232_native
         * class.
         */
        std::vector<std::shared_ptr<RS232_native>> rs232Devices;

        /**
         * @brief The index of the current device that is used for the connection
         *
         * This index usually refers to the first connected device.
         * If no device is connected the index can be anything however if possible it is a device that was connected before or is in theory
         * available on the current OS.
         */
        std::atomic<size_t> currentDevice = 0;

      public:
        /**
         * @brief Construct a new RS232 object with a single device
         * @note this constructor calls getAvailablePorts() to get a list of available ports and then calls the constructor with a list of
         * devices.
         *
         * @param Rate The  rate that should be used for the connection
         * @param errorStream The stream where error messages should be written to
         */
        [[maybe_unused]]
        explicit RS232(Baudrate Rate, std::ostream& errorStream = std::cerr);

        /**
         * @brief Construct a new RS232 object with a single device
         * @note this constructor just calls the one with a list of devices with a list size of 1.
         *
         * @param deviceName The name of the port where the device is connected to
         * @param Rate The  rate that should be used for the connection
         * @param errorStream The stream where error messages should be written to
         */
        [[maybe_unused]]
        RS232(const std::string& deviceName, Baudrate Rate, std::ostream& errorStream = std::cerr);

        /**
         * @brief Construct a new RS232 object with a list of devices
         *
         * @param deviceNames The list of names of the ports where the device may be connected to
         * @param baudrate The baudrate that should be used for the connection
         * @param errorStream The stream where error messages should be written to
         */
        [[maybe_unused]]
        RS232(const std::vector<std::string>& deviceNames, Baudrate baudrate, std::ostream& errorStream = std::cerr);

        /**
         * @brief Destroy the RS232.
         * This function calls the Close() function automatically to leave the port in a valid state.
         *
         */
        ~RS232();

        /**
         * @brief Get a pointer to a native device
         * If the index is not a valid index the current device will be returned.
         *
         * @param index the index of the device that should be returned
         */
        [[nodiscard]] [[maybe_unused]]
        std::shared_ptr<RS232_native> getNativeDevice(size_t index = -1) const;

        /**
         * @brief Get the number of devices that are added to this class
         * @return The size of the list of devices
         */
        [[nodiscard]] [[maybe_unused]]
        size_t getDeviceCount() const;

        /**
         * @brief Checks if the connection was started successfully
         *
         * @return true the connection is established as expected
         * @return false there was an error while initializing the connection or some of the settings are not valid
         */
        [[nodiscard]] [[maybe_unused]]
        bool IsAvailable(size_t index = -1) const;

        /**
         * @brief Get the name of the port used for this RS232 connection
         * If the index is not a valid index the current device will be checked.
         *
         * @param index the index of the device that should be used
         */
        [[nodiscard]] [[maybe_unused]]
        std::string_view GetDeviceName(size_t index = -1) const;

        /**
         * @brief reads until the next character is received
         *
         * @return std::tuple<unsigned char, int> this tuple contains the wanted return data and an error code in case something went wrong
         * The return value is >= 0 if everything is okay and < 0 if something went wrong
         */
        [[nodiscard]] [[maybe_unused]]
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
        template <class Rep = int64_t, class Period = std::ratio<1>>
        [[nodiscard]] [[maybe_unused]]
        std::tuple<unsigned char, int> ReadNextChar(std::chrono::duration<Rep, Period> waitTime,
                                                    bool                               ignoreTime     = false,
                                                    std::shared_ptr<RS232_native>      transferDevice = nullptr);

        /**
         * @brief reads the interface until a newline (\n) is received
         *
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        [[nodiscard]] [[maybe_unused]]
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
        template <class Rep = int64_t, class Period = std::ratio<1>>
        [[nodiscard]] [[maybe_unused]]
        std::tuple<std::string, int> ReadNextMessage(std::chrono::duration<Rep, Period> waitTime, bool ignoreTime = false);

        /**
         * @brief read the interface until one of the stop conditions is reached
         *
         * @param conditions a vector containing all the stop conditions.
         * @return std::tuple<std::string, int> this tuple contains the wanted return data and an error code in case something went wrong
         */
        [[nodiscard]] [[maybe_unused]]
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
        template <class Rep = int64_t, class Period = std::ratio<1>>
        [[nodiscard]] [[maybe_unused]]
        std::tuple<std::string, int>
        ReadUntil(const std::vector<unsigned char>& conditions, std::chrono::duration<Rep, Period> waitTime, bool ignoreTime = false);

        /**
         * @brief print a string to the currently connected device
         * @param text the test to send
         */
        [[maybe_unused]]
        void Print(const std::string& text, std::ostream& errorStream = std::cerr);

        /**
         * @brief close the connection to the device.
         * @warning this disables all following transactions to the device.
         *
         */
        [[deprecated("use DisconnectAll() instead")]]
        void Close();

        [[maybe_unused]]
        bool Connect();

        [[maybe_unused]]
        void DisconnectAll();

        /**
         * @brief check if the clear to send flag is set
         *
         * @return true if the flag is set
         * @return false if the flag is not set
         */
        [[nodiscard]] [[maybe_unused]]
        bool IsCTSEnabled() const;
    };

} // namespace sakurajin

#include "rs232_template_implementations.hpp"

#endif // SAKURAJIN_RS232_HPP_INCLUDED
