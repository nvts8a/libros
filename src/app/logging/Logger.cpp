#include "Logger.h"
#include <chrono>

Logger::Logger() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice(); // TODO: Maybe make this part of device since it's dependant on it
    if (device->fileExists(logFilename)) {
        logFile = device->openFile(logFilename, O_RDWR | O_APPEND);
    } else logFile = device->openFile(logFilename, O_RDWR | O_CREAT);
}

void Logger::debug(std::string logLine) {
    if (logLevel <= 1) Logger::_log(levelDebug + logLine);
}

void Logger::info(std::string logLine) {
    if (logLevel <= 2) Logger::_log(levelInfo + logLine);
}

void Logger::warn(std::string logLine) {
    if (logLevel <= 3) Logger::_log(levelWarn + logLine);
}

void Logger::error(std::string logLine) {
    Logger::_log(levelError + logLine);
}

void Logger::_log(std::string logLine) {
    std::string timedLogLine = _getTimestamp() + logLine;
    for(int i = 0; i < timedLogLine.size(); i++) logFile.write(timedLogLine[i]);
    logFile.write('\n'); logFile.flush();
}

std::string Logger::_getTimestamp() {
    char timestamp[20];

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::strftime(timestamp, 20, "%FT%T", std::localtime(&now_c));

    return std::string(timestamp);
}