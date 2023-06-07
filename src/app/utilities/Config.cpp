#include "Config.h"

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
    switch (keyValue(key)) {
        case (KEY::LogDebug):
            if (value.toInt() == 1) this->configuration.logDebug = true;
            break;
        case (KEY::LogTruncate):
            if (value.toInt() == 1) this->configuration.logTruncate = true;
            break;
        default:
            break;
    }
}