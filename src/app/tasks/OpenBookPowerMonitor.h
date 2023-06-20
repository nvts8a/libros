#ifndef OpenBookPowerMonitor_h
#define OpenBookPowerMonitor_h

#include "Application.h"

class OpenBookPowerMonitor : public Task {
public:
    OpenBookPowerMonitor() {};
    bool run(std::shared_ptr<Application> application);
protected:
    int8_t wasOnBattery = -1;
};

#endif // OpenBookPowerMonitor_h