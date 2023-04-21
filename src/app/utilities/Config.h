#ifndef Config_h
#define Config_h

#include <string>
#include "OpenBookDevice.h"

static const std::string LOG_DEBUG_KEY = "LOG.DEBUG";
static const std::string LOG_TRUNCATE_KEY = "LOG.TRUNCATE";

class Config {
public:
    static Config *config() {
        static Config instance;
        return &instance;
    }

    static std::string SOFTWARE_VERSION() {
        return "v0.7.4";
    }

    static bool DEBUG_LOG_LEVEL_ENABLED() {
        return Config::config()->_getLogDebug();
    }

    static bool TRUNCATE_LOG_FILES_ENABLED() {
        return Config::config()->_getLogTruncate();
    }

protected:
    bool _getLogDebug();
    bool _getLogTruncate();
    void _parseConfigFile();

    bool logDebug = false;
    bool logTruncate = false;
private:
    Config();
};


#endif // Config_h