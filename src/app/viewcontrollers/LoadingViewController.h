#ifndef LoadingViewController_h
#define LoadingViewController_h

#include "Application.h"
#include "Widgets.h"
#include "Config.h"
#include "bitmaps.h"

static const Rect MESSAGE_RECT = MakeRect(22, 300, 256, 8);
static const std::shared_ptr<Label> VERSION = std::make_shared<Label>(MakeRect(300-SOFTWARE_VERSION.length()*6, 392, SOFTWARE_VERSION.length()*6, 8), SOFTWARE_VERSION);
static const std::shared_ptr<BitmapView> SPLASH = std::make_shared<BitmapView>(MakeRect(0, 0, 300, 400), OpenBookSplash);

class LoadingViewController : public ViewController {
public:
    LoadingViewController(std::shared_ptr<Application> application, std::string defaultMessage);
    void setMessage(std::string message);
protected:
    virtual void createView() override;
    std::string defaultMessage;
    std::shared_ptr<Label> messageLabel;
};

#endif // LoadingViewController_h