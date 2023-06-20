#ifndef OpenBookRawButtonInput_h
#define OpenBookRawButtonInput_h

#include "Application.h"

class OpenBookRawButtonInput : public Task {
public:
    OpenBookRawButtonInput() {};
    bool run(std::shared_ptr<Application> application);
protected:
    uint8_t lastButtons = 0;
};

#endif // OpenBookRawButtonInput_h