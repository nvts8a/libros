#ifndef Config_h
#define Config_h

#include <string>
#include "OpenBookDevice.h"

static const std::string SOFTWARE_VERSION = "v0.9.4"; // SED-BUOY
static const std::string CONF_DIR  = "/_OPENBOOK/";
static const std::string CONF_FILE = "/_OPENBOOK/openbook.conf";

// Struct of all configuration keys available to use
typedef struct {
    arduino::String i18nTag     = "I18N.TAG";
    arduino::String logDebug    = "LOG.DEBUG";
    arduino::String logTruncate = "LOG.TRUNCATE";
} Keys;

// Configuration struct for storing all the types of config nonsense used by the running app
typedef struct {
    bool logDebug = false;
    bool logTruncate = false;
    arduino::String i18nTag = "en";
} Configuration;

class Config {
public:
    static Config *config() {
        static Config instance;
        return &instance;
    }

    static bool DEBUG_LOG_LEVEL_ENABLED() {
        return Config::config()->_getLogDebug();
    }

    static bool TRUNCATE_LOG_FILES_ENABLED() {
        return Config::config()->_getLogTruncate();
    }

    static arduino::String I18N_TAG() {
        return Config::config()->_getI18nTag();
    }

protected:
    const Keys keys;
    bool _getLogDebug();
    bool _getLogTruncate();
    arduino::String _getI18nTag();
    void _parseConfigFile();
    void _setConfiguration(arduino::String key, arduino::String value);
    Configuration configuration;
private:
    Config();
};


#endif // Config_h