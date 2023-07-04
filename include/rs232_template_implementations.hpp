#pragma once

#ifdef SAKURAJIN_RS232_HPP_INCLUDED

template <class Rep, class Period>
std::tuple<unsigned char, int>
sakurajin::RS232::ReadNextChar(std::chrono::duration<Rep, Period> waitTime, bool ignoreTime, std::shared_ptr<RS232_native> transferDevice) {
    if (transferDevice == nullptr) {
        transferDevice = rs232Devices[currentDevice];
    }

    if (transferDevice->getConnectionStatus() != sakurajin::connectionStatus::connected) {
        return {'\0', -1};
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    char IOBuf   = '\0';
    int  errCode = 0;

    do {
        errCode = transferDevice->readRawData(&IOBuf, 1);

        if (ignoreTime) {
            continue;
        }

        if (std::chrono::high_resolution_clock::now() - startTime > waitTime) {
            return {'\0', -2};
        }
    } while (errCode != 1);

    return {IOBuf, 0};
}


template <class Rep, class Period>
std::tuple<std::string, int> sakurajin::RS232::ReadNextMessage(std::chrono::duration<Rep, Period> waitTime, bool ignoreTime) {
    return ReadUntil({'\n'}, waitTime, ignoreTime);
}

template <class Rep, class Period>
std::tuple<std::string, int>
sakurajin::RS232::ReadUntil(const std::vector<unsigned char>& conditions, std::chrono::duration<Rep, Period> waitTime, bool ignoreTime) {
    auto transferDevice = rs232Devices[currentDevice];
    if (transferDevice->getConnectionStatus() != sakurajin::connectionStatus::connected) {
        return {"", -1};
    }

    std::string message  = "";
    int         errCode  = 0;
    char        nextChar = '\n';
    bool        stop     = false;

    while (!stop) {
        // read the next char and append if there was no error
        std::tie(nextChar, errCode) = ReadNextChar(waitTime, ignoreTime, transferDevice);

        if (errCode < 0) {
            return {"", -1};
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

#else

    #error "do not directly include this header!"

#endif
