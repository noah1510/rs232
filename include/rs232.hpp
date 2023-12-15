#ifndef SAKURAJIN_RS232_HPP_INCLUDED
#define SAKURAJIN_RS232_HPP_INCLUDED

#include "rs232_native.hpp"

#include <future>

namespace sakurajin {

    /**
     * @brief The RS232 class is a wrapper class for the RS232_native class.
     * It can contain many RS232_native objects and allows the user to switch between them.
     * The raw read/write functions are abstracted into more practical interfaces.
     *
     * Just like the RS232_native class this class is supposed to be thread safe.
     * If something is not then that is treated as a bug.
     */
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

        std::shared_mutex deviceMutex;

        /**
         * @brief The work thread that actually does most of the work
         */
        std::future<void> workThread;
        std::atomic<bool> stopThread = false;

        std::string       readBuffer;
        std::timed_mutex  readBufferMutex;
        std::atomic<bool> readBufferHasData = false;

        std::string       writeBuffer;
        std::timed_mutex  writeBufferMutex;
        std::atomic<bool> writeBufferHasData = false;

        /**
         * @brief The function that is executed by the work thread
         * It performs the actual read/write operations constantly in the background.
         * This function only does one loop iteration and then returns.
         * It should be called in a loop to keep the query system running.
         */
        void work();

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
         * @brief connect to the first available device
         *
         * @return true if the connection was established successfully
         * @return false if the connection could not be established
         */
        [[maybe_unused]]
        bool Connect();

        /**
         * @brief disconnect from all devices
         * If no device is connected this will exit early otherwise it will disconnect all devices.
         */
        [[maybe_unused]]
        void DisconnectAll();

        /**
         * @brief Get a pointer to a native device
         * If the index is not a valid index the current device will be returned.
         *
         * @param index the index of the device that should be returned
         */
        [[nodiscard]] [[maybe_unused]]
        std::shared_ptr<RS232_native> getNativeDevice(size_t index) const;

        /**
         * @brief Get a pointer to the current native device
         * This is the same as calling getNativeDevice(currentDevice) but is more convenient since its used more often.
         */
        [[nodiscard]] [[maybe_unused]]
        std::shared_ptr<RS232_native> getCurrentDevice() const;

        /**
         * @brief check if the current device is available
         * @return bool true if the current device is connected and ready to use
         */
        [[nodiscard]] [[maybe_unused]]
        bool IsAvailable() const;

        /**
         * @brief Get the number of devices that are added to this class
         * @return The size of the list of devices
         */
        [[nodiscard]] [[maybe_unused]]
        size_t getDeviceCount() const;

        /**
         * @brief Empty the read buffer and return its content
         * @warning the read buffer will be cleared after this function is called.
         * @return std::string the content of the read buffer
         */
        [[nodiscard]] [[maybe_unused]]
        std::string retrieveReadBuffer();

        /**
         * @brief load the read buffer and return the first match with a regex
         * This function uses the std::regex_search function to find the first match of the read buffer.
         * If no match is found an empty string is returned.
         * @note this function clears the read buffer until the end of the match.
         * If the buffer contains "Hello World!" and the pattern is "World" the buffer will be cleared until the end of the match.
         * The buffer will then contain "!". Everything in front of the match will be discarded.
         * @param pattern the regex pattern that should be used
         * @return std::string the first match of the read buffer
         */
        [[nodiscard]] [[maybe_unused]]
        std::string retrieveFirstMatch(const std::regex& pattern);

        /**
         * @brief load the read buffer and return all matches with a regex
         * This does essentially the same as retrieveFirstMatch but returns all matches instead of just the first one.
         * If care about more than one result use this function since it is more efficient than calling retrieveFirstMatch multiple
         * times. If no match is found or no data is in the read buffer, an empty vector is returned.
         * @note this function clears the read buffer until the end of the last match.
         * @note The read buffer is not updated during this function call. If the read buffer is large and many pattern matches are
         * found this function might take a long time to complete. During this time more data might be queued for adding to the
         * read buffer. This data will not be considered by this function.
         * @param pattern the regex pattern that should be used
         * @return std::vector<std::string> all matches of the read buffer
         */
        [[nodiscard]] [[maybe_unused]]
        std::vector<std::string> retrieveAllMatches(const std::regex& pattern);

        /**
         * @brief print a string to the currently connected device
         * This function adds the string to the write buffer and then returns.
         * The write buffer is then written to the device in the background by the work thread.
         * Because of this the actual write operation might be delayed.
         * For a more immediate write operation use the native device directly.
         *
         * @param text the text to send
         */
        [[maybe_unused]]
        void Print(std::string text);


        /**
         * @brief check if the clear to send flag is set
         *
         * @return true if the flag is set
         * @return false if the flag is not set
         */
        [[nodiscard]] [[maybe_unused]] [[deprecated("get the native device and retrieve the information from there")]]
        bool IsCTSEnabled() const;

        /**
         * @brief Checks if the connection was started successfully
         *
         * @return true the connection is established as expected
         * @return false there was an error while initializing the connection or some of the settings are not valid
         */
        [[nodiscard]] [[maybe_unused]] [[deprecated("get the native device and retrieve the information from there")]]
        bool IsAvailable(size_t index) const;

        /**
         * @brief Get the name of the port used for this RS232 connection
         * If the index is not a valid index the current device will be checked.
         *
         * @param index the index of the device that should be used
         */
        [[nodiscard]] [[maybe_unused]] [[deprecated("use getNativeDevice(index)->getDeviceName() instead")]]
        std::string_view GetDeviceName(size_t index) const;

        /**
         * @brief close the connection to the device.
         * @warning this disables all following transactions to the device.
         *
         */
        [[deprecated("use DisconnectAll() instead")]]
        void Close();
    };

} // namespace sakurajin

#endif // SAKURAJIN_RS232_HPP_INCLUDED
