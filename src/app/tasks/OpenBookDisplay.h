#ifndef OpenBookDisplay_h
#define OpenBookDisplay_h

#include "OpenBook_IL0398.h"
#include "Application.h"

class OpenBookDisplay : public Task {
public:
    OpenBookDisplay();
    bool run(std::shared_ptr<Application> application);
protected:
    void splash();
    OpenBook_IL0398 *display;
};


#endif // OpenBookDisplay_h