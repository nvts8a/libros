#ifndef OpenBookDisplay_h
#define OpenBookDisplay_h

#include "OpenBook_IL0398.h"
#include "Application.h"

class OpenBookDisplay : public Task {
public:
    OpenBookDisplay();
    bool run(std::shared_ptr<Application> application);
    void display();
    void display(OpenBookDisplayMode displayMode);
    void displayPartial(Rect dirtyRect);
    void displayPartial(OpenBookDisplayMode displayMode, Rect dirtyRect);
protected:
    OpenBook_IL0398 *displayDevice;
};


#endif // OpenBookDisplay_h