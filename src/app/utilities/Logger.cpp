#include "Logger.h"
#include "Config.h"

Logger::Logger() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    if (!device->fileExists("/_OPENBOOK")) device->makeDirectory("/_OPENBOOK");
    std::string logFilename = "/_OPENBOOK/openbook.log";

    if (device->fileExists(logFilename.c_str())) {
        if (Config::TRUNCATE_LOG_FILES_ENABLED()) {
            device->removeFile(logFilename.c_str());
            this->logFile = device->openFile(logFilename.c_str(), O_RDWR | O_CREAT);
        } else this->logFile = device->openFile(logFilename.c_str(), O_RDWR | O_APPEND);
    } else this->logFile = device->openFile(logFilename.c_str(), O_RDWR | O_CREAT);

    if (this->logFile.isOpen()) this->_printBanner();
}

/**
 * Used by the static method DEBUG to check for the debug flag, construct and write a DEBUG log message.
 * @param logLine The string contents of the DEBUG message
*/
void Logger::_debug(std::string logLine) {
    if (Config::DEBUG_LOG_LEVEL_ENABLED()) this->_log(DEBUG_PREFIX + logLine);
}

/**
 * Used by the static method INFO to construct and write an INFO log message.
 * @param logLine The string contents of the INFO message
*/
void Logger::_info(std::string logLine) {
    this->_log(INFO_PREFIX + logLine);
}

/**
 * Used by the static method WARN to construct and write an WARN log message.
 * @param logLine The string contents of the WARN message
*/
void Logger::_warn(std::string logLine) {
    this->_log(WARN_PREFIX + logLine);
}

/**
 * Used by the static method ERROR to construct and write an ERROR log message.
 * @param logLine The string contents of the ERROR message
*/
void Logger::_error(std::string logLine) {
    this->_log(ERROR_PREFIX + logLine);
}

/**
 * If in DEBUG mode, starts or stops a load test, which will report back between then the wall time
 *  in nanoseconds between the two events as DEBUG logs.
 * Otherwise will do nothing.
*/
void Logger::_loadTest() {
    if (Config::DEBUG_LOG_LEVEL_ENABLED()) {
        if (this->loadStart) {
            uint32_t elapsedTime = to_ms_since_boot(get_absolute_time()) - this->loadStart;
            this->loadStart = 0;
            this->_debug("Load test completed and took: " + std::to_string(elapsedTime) + "ms");
        } else {
            this->loadStart = to_ms_since_boot(get_absolute_time());
            this->_debug("Load test starting...");
        }
    }
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
 * A fun banner cause this is a fun project.
*/
void Logger::_printBanner() {
    this->_log("");
    if(Config::DEBUG_LOG_LEVEL_ENABLED()) {
        this->_log("@@@       @@@  @@@@@@@   @@@@@@@    @@@@@@    @@@@@@");
        this->_log("@@@       @@@  @@@@@@@@  @@@@@@@@  @@@@@@@@  @@@@@@@ ");
        this->_log("@@!       @@!  @@!  @@@  @@!  @@@  @@!  @@@  !@@  ");
        this->_log("!@!       !@!  !@   @!@  !@!  @!@  !@!  @!@  !@! ");
        this->_log("@!!       !!@  @!@!@!@   @!@!!@!   @!@  !@!  !!@@!! ");
        this->_log("!!!       !!!  !!!@!!!!  !!@!@!    !@!  !!!   !!@!!! ");
        this->_log("!!:       !!:  !!:  !!!  !!: :!!   !!:  !!!       !:! ");
        this->_log("!:!       :!:  :!:  !:!  :!:  !:!  :!:  !:!      !:! ");
        this->_log(" :: ::::   ::   :: ::::  ::   :::  ::::: ::  :::: ::    " + SOFTWARE_VERSION);
        this->_log(": :: : :  : :  :: : ::   : :  : :   : :  :   :: : :     DEBUG MODE");
    } else {
        this->_log(" /./_ __   _");
        this->_log("///_///_/_\\  " + SOFTWARE_VERSION);
    }
    this->_log("");
}