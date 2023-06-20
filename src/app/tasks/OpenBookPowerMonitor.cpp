#include "OpenBookPowerMonitor.h"
#include "OpenBookDevice.h"
#include "OpenBookEvents.h"

bool OpenBookPowerMonitor::run(std::shared_ptr<Application> application) {
    float systemVoltage = OpenBookDevice::sharedDevice()->getSystemVoltage();
    bool onBattery = systemVoltage < 4.5;

    if (onBattery != this->wasOnBattery) {
        application->generateEvent(OPEN_BOOK_EVENT_POWER_CHANGED, (int32_t) (systemVoltage * 100));
    }
    this->wasOnBattery = onBattery;

    return false;
}