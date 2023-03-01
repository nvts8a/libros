#include "Logger.h"
#include <chrono>

Logger::Logger() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    if (!device->fileExists("/.logs")) device->makeDirectory("/.logs");

    std::string logFilename = "/.logs/.openbook.log";
    if (device->fileExists(logFilename.c_str())) {
        logFile = device->openFile(logFilename.c_str(), O_RDWR | O_APPEND);
    } else logFile = device->openFile(logFilename.c_str(), O_RDWR | O_CREAT);

    if (logFile.isOpen()) this->_printBanner();
}

/**
 * Used by the static method DEBUG to check for the debug flag, construct and write a DEBUG log message.
 * @param logLine The string contents of the DEBUG message
*/
void Logger::_debug(std::string logLine) {
    if (this->debug) this->_log(_getTimestamp() + DEBUG_PREFIX + logLine);
}

/**
 * Used by the static method INFO to construct and write an INFO log message.
 * @param logLine The string contents of the INFO message
*/
void Logger::_info(std::string logLine) {
    this->_log(_getTimestamp() + INFO_PREFIX + logLine);
}

/**
 * Used by the static method WARN to construct and write an WARN log message.
 * @param logLine The string contents of the WARN message
*/
void Logger::_warn(std::string logLine) {
    this->_log(_getTimestamp() + WARN_PREFIX + logLine);
}

/**
 * Used by the static method ERROR to construct and write an ERROR log message.
 * @param logLine The string contents of the ERROR message
*/
void Logger::_error(std::string logLine) {
    this->_log(_getTimestamp() + ERROR_PREFIX + logLine);
}

/**
 * Generic method used for writing log messages to the open log file.
 * @param logLine The string contents of the log message
*/
void Logger::_log(std::string logLine) {
    for(uint16_t i = 0; i < logLine.size(); i++) logFile.write(logLine[i]);
    logFile.write('\n'); logFile.flush();
}

/**
 * Constructs a ISO 8601 Timestamp string used for the beginning of log level log messages
 * @return The current timestamp string in ISO 8601 format
*/
std::string Logger::_getTimestamp() {
    char timestamp[20];

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::strftime(timestamp, 20, "%FT%T", std::localtime(&now_c));

    return std::string(timestamp);
}

/**
 * A fun banner cause this is a fun project.
*/
void Logger::_printBanner() {
    this->_log("");
    if(this->debug) {
        this->_log("@@@       @@@  @@@@@@@   @@@@@@@    @@@@@@    @@@@@@");
        this->_log("@@@       @@@  @@@@@@@@  @@@@@@@@  @@@@@@@@  @@@@@@@ ");
        this->_log("@@!       @@!  @@!  @@@  @@!  @@@  @@!  @@@  !@@  ");
        this->_log("!@!       !@!  !@   @!@  !@!  @!@  !@!  @!@  !@! ");
        this->_log("@!!       !!@  @!@!@!@   @!@!!@!   @!@  !@!  !!@@!! ");
        this->_log("!!!       !!!  !!!@!!!!  !!@!@!    !@!  !!!   !!@!!! ");
        this->_log("!!:       !!:  !!:  !!!  !!: :!!   !!:  !!!       !:! ");
        this->_log("!:!       :!:  :!:  !:!  :!:  !:!  :!:  !:!      !:! ");
        this->_log(" :: ::::   ::   :: ::::  ::   :::  ::::: ::  :::: ::    v0.5.3");
        this->_log(": :: : :  : :  :: : ::   : :  : :   : :  :   :: : :     DEBUG MODE");
    } else {
        this->_log(" /./_ __   _");
        this->_log("///_///_/_\\  v0.5.3");
    }
    this->_log("");
}