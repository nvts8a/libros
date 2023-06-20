#ifndef OpenBookLockScreen_h
#define OpenBookLockScreen_h

#include "Application.h"

class OpenBookLockScreen : public Task {
public:
    OpenBookLockScreen() {};
    bool run(std::shared_ptr<Application> application);
};

#endif // OpenBookLockScreen_h