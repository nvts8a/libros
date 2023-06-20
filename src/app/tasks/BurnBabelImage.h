#ifndef BurnBabelImage_h
#define BurnBabelImage_h

#include "Adafruit_SPIFlash.h"
#include "Application.h"

class BurnBabelImage : public Task {
public:
    BurnBabelImage() {};
    bool run(std::shared_ptr<Application> application);
protected:
    File babelFile;
    int32_t page = -1;
    int32_t numPages = 0;
    int32_t lastUpdate = -1;
};

#endif // BurnBabelImage_h
