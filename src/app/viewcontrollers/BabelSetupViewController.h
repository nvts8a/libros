#ifndef BabelSetupViewController_h
#define BabelSetupViewController_h

#include "Application.h"
#include "Widgets.h"

class BabelSetupViewController : public ViewController {
public:
    BabelSetupViewController(std::shared_ptr<Application> application) : ViewController(application) {};
    void dismiss(Event event);
protected:
    virtual void createView() override;
    void updateProgress(Event event);
    std::shared_ptr<ProgressView> progressView;
};

#endif // BabelSetupViewController_h
