#ifndef Logger_h
#define Logger_h

#include <string>
#include "OpenBookDevice.h"

static const char* logFilename = "/.openbook.log";
static const uint16_t logLevel = 2; // TODO: have this app conf configurable
static const std::string levelDebug = " [DEBUG] ";
static const std::string levelInfo  = " [INFO]  ";
static const std::string levelWarn  = " [WARN]  ";
static const std::string levelError = " [ERROR] ";

class Logger {
public:
    static Logger *l() {
        static Logger instance;
        return &instance;
    }
    void debug(std::string logLine);
    void info(std::string logLine);
    void warn(std::string logLine);
    void error(std::string logLine);
protected:
    void _log(std::string logLine);
    std::string _getTimestamp();
    File logFile;
private:
    Logger();
};


#endif // Logger_h