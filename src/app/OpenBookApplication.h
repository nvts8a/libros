#ifndef OpenBookApplication_h
#define OpenBookApplication_h

#include "Application.h"
#include "Widgets.h"
#include "OpenBookDatabase.h"
#include "OpenBookDisplay.h"
#include "LoadingViewController.h"
#include "BookListViewController.h"
#include <memory>

class OpenBookApplication : public Application {
public:
    OpenBookApplication() : Application(300, 400) {};

    void setup() override;

    bool locked = false;

    std::shared_ptr<BookListViewController> mainMenu;
    std::shared_ptr<LoadingViewController> loadingView;
    void setLoadingMessage(std::string message);

    // Callbacks
    void showLockScreen(Event event);
    void showBookReader(Event event);
    void returnHome(Event event);
    void changeRefreshMode(Event event);

    int requestedRefreshMode = -1;

protected:
    std::shared_ptr<OpenBookDisplay> displayTask;
};

#endif // OpenBookApplication_h
