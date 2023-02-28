#include "Logger.h"
#include <chrono>

Logger::Logger() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice(); // TODO: Maybe make this part of device since it's dependant on it
    //const char logFilename[14] = { '/', '.', 'o', 'p', 'e', 'n', 'b', 'o', 'o', 'k', '.', 'l', 'o', 'g' };
    std::string logFilename = "/.openbook.log";
    if (device->fileExists(logFilename.c_str())) {
        logFile = device->openFile(logFilename.c_str(), O_RDWR | O_APPEND);
    } else logFile = device->openFile(logFilename.c_str(), O_RDWR | O_CREAT);
}

void Logger::debug(std::string logLine) {
    if (this->logLevel <= 1) Logger::_log(DEBUG + logLine);
}

void Logger::info(std::string logLine) {
    if (this->logLevel <= 2) Logger::_log(INFO + logLine);
}

void Logger::warn(std::string logLine) {
    if (this->logLevel <= 3) Logger::_log(WARN + logLine);
}

void Logger::error(std::string logLine) {
    Logger::_log(ERROR + logLine);
}

void Logger::_log(std::string logLine) {
    std::string timedLogLine = _getTimestamp() + logLine;
    for(uint16_t i = 0; i < timedLogLine.size(); i++) logFile.write(timedLogLine[i]);
    logFile.write('\n'); logFile.flush();
}

std::string Logger::_getTimestamp() {
    char timestamp[20];

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::strftime(timestamp, 20, "%FT%T", std::localtime(&now_c));

    return std::string(timestamp);
}