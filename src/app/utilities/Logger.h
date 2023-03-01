#ifndef Logger_h
#define Logger_h

#include <string>
#include "OpenBookDevice.h"
#include "Config.h"

static const std::string DEBUG_PREFIX = " [DEBUG] ";
static const std::string INFO_PREFIX  = " [INFO]  ";
static const std::string WARN_PREFIX  = " [WARN]  ";
static const std::string ERROR_PREFIX = " [ERROR] ";

class Logger {
public:
    static Logger *logger() {
        static Logger instance;
        return &instance;
    }
    static void DEBUG(std::string logLine) {
        Logger::logger()->_debug(logLine);
    }
    static void INFO(std::string logLine) {
        Logger::logger()->_info(logLine);
    }
    static void WARN(std::string logLine) {
        Logger::logger()->_warn(logLine);
    }
    static void ERROR(std::string logLine) {
        Logger::logger()->_error(logLine);
    }

protected:
    void _debug(std::string logLine);
    void _info(std::string logLine);
    void _warn(std::string logLine);
    void _error(std::string logLine);
    void _printBanner();
    void _log(std::string logLine);
    std::string _getTimestamp();

    File logFile;
private:
    Logger();
};


#endif // Logger_h