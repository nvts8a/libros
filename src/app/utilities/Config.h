#ifndef Config_h
#define Config_h

#include <string>
#include "OpenBookDevice.h"

static const std::string CONF_DIR  = "/_OPENBOOK/";
static const std::string CONF_FILE = "/_OPENBOOK/openbook.conf";

// Nonsense so I can use a switch state with strings
enum KEY { LogDebug, LogTruncate, InvalidKey };
static KEY keyValue(arduino::String key) {
    if (key.compareTo("LOG.DEBUG") == 0)    return KEY::LogDebug;
    if (key.compareTo("LOG.TRUNCATE") == 0) return KEY::LogTruncate;
    return KEY::InvalidKey;
}

// Configuration struct for storing all the types of config nonsense used by the running app
typedef struct {
    bool logDebug = false;
    bool logTruncate = false;
} Configuration;

class Config {
public:
    static Config *config() {
        static Config instance;
        return &instance;
    }

    static std::string SOFTWARE_VERSION() {
        return "v0.8.3"; // SED-BUOY
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
    void _setConfiguration(arduino::String key, arduino::String value);
    Configuration configuration;
private:
    Config();
};


#endif // Config_h