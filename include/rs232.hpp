#ifndef SAKURAJIN_RS232_HPP_INCLUDED
#define SAKURAJIN_RS232_HPP_INCLUDED

#include "rs232_native.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace sakurajin {

    class RS232_EXPORT_MACRO RS232 {
      private:
        std::vector<std::shared_ptr<RS232_native>> rs232Devices;
        std::atomic<size_t>                        currentDevice = 0;

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
        template <class Rep = int64_t, class Period = std::ratio<1>>
        std::tuple<unsigned char, int> ReadNextChar(std::chrono::duration<Rep, Period> waitTime,
                                                    bool                               ignoreTime     = false,
                                                    std::shared_ptr<RS232_native>      transferDevice = nullptr);

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
        template <class Rep = int64_t, class Period = std::ratio<1>>
        std::tuple<std::string, int> ReadNextMessage(std::chrono::duration<Rep, Period> waitTime, bool ignoreTime = false);

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
        template <class Rep = int64_t, class Period = std::ratio<1>>
        std::tuple<std::string, int>
        ReadUntil(const std::vector<unsigned char>& conditions, std::chrono::duration<Rep, Period> waitTime, bool ignoreTime = false);

        /**
         * @brief print a string to the currently connected device
         * @param text the test to send
         */
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

} // namespace sakurajin

#include "rs232_template_implementations.hpp"

#endif // RS232_HPP_INCLUDED
