#ifndef Logger_h
#define Logger_h

#include <string>
#include "OpenBookDevice.h"

static const std::string DEBUG = " [DEBUG] ";
static const std::string INFO  = " [INFO]  ";
static const std::string WARN  = " [WARN]  ";
static const std::string ERROR = " [ERROR] ";

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
    uint16_t logLevel = 2; // TODO: have this app conf configurable
private:
    Logger();
};


#endif // Logger_h