#include "Config.h"
#include "I18n.h"
#include <algorithm>

Config::Config() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    if (!device->fileExists(CONF_DIR.c_str())) device->makeDirectory(CONF_DIR.c_str());
    this->_parseConfigFile();
}

/**
 * Used to determine if the app is running logs in debug mode
 * @return True if the app is in debug
*/
bool Config::_getLogDebug() {
    return this->configuration.logDebug;
}

/**
 * Used to determine if the app truncates the log file on every boot
 * @return True if the app will truncate the log
*/
bool Config::_getLogTruncate() {
    return this->configuration.logTruncate;
}

/**
 * Used to retrieve the current I18N Tag. This should be the teo character primary tag of a IS0 639-1 code
 * @return A two character lowercase string, IS0 639-1 primary tag, of a major language currently configured
*/
arduino::String Config::_getI18nTag() {
    return this->configuration.i18nTag;
}

/**
 * Parses the file at CONF_FILE location. Expects the keys to be seperated from the 
 *  values by the char '='. The keys are not case sensative, any invalid keys are ignored.
*/
void Config::_parseConfigFile() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();

    if (device->fileExists(CONF_FILE.c_str())) {
        File configFile = device->openFile(CONF_FILE.c_str(), O_RDONLY);

        while (configFile.available()) {
            arduino::String key = configFile.readStringUntil('='); key.toUpperCase(); // Get the key and uppercase it
            while (configFile.peek() == ' ') configFile.read();                       // Ignore any whitespace
            arduino::String value = configFile.readStringUntil('\n');                 // Get the value until you get a new line

            this->_setConfiguration(key, value);
        }

        configFile.close();
    } else { // if a file doesn't exist just create an empty one
        File emptyConfFile = device->openFile(CONF_FILE.c_str(), O_CREAT | O_RDWR);
        emptyConfFile.flush(); emptyConfFile.close();
    }
}

/**
 * A helper function to contain the messy key checking and value setting logic
 *  from the parsing code
 * @param key An arduino string representing the type of the configuration
 * @param value An arduino string representing what the configuration is to be set to
*/
void Config::_setConfiguration(arduino::String key, arduino::String value) {
    if (keys.i18nTag == key) {
        value.toLowerCase();
        bool supportedLanguage = std::find(LANGUAGES.begin(), LANGUAGES.end(), value) != LANGUAGES.end();
        if (supportedLanguage) this->configuration.i18nTag = value;
    }
    else if (keys.logDebug    == key) {
        if (value.toInt() == 1) configuration.logDebug = true;
    }
    else if (keys.logTruncate == key) {
        if (value.toInt() == 1) configuration.logTruncate = true;
    }
}