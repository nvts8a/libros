#ifndef CoverViewController_h
#define CoverViewController_h

#include "Application.h"

class CoverViewController : public ViewController {
public:
    CoverViewController(std::shared_ptr<Application> application); 
protected:
    virtual void createView() override;
};

#endif // CoverViewController_h