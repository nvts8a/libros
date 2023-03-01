#include "Config.h"

Config::Config() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    if (!device->fileExists("/_OPENBOOK")) device->makeDirectory("/_OPENBOOK");
    this->_parseConfigFile();
}

/**
 * 
*/
bool Config::_getLogDebug() {
    return this->logDebug;
}

/**
 * 
*/
bool Config::_getLogTruncate() {
    return this->logTruncate;
}

/**
 * 
*/
void Config::_parseConfigFile() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    std::string configFilename = "/_OPENBOOK/openbook.conf";

    if (device->fileExists(configFilename.c_str())) {
        this->logDebug = true;
    }
}