#ifndef OpenBookApplication_h
#define OpenBookApplication_h

#include "Application.h"
#include "Widgets.h"
#include "OpenBookDatabase.h"
#include "BookListViewController.h"
#include <memory>

class OpenBookApplication : public Application {
public:
    OpenBookApplication() : Application(300, 400) {};

    void setup() override;

    bool locked = false;

    std::shared_ptr<BookListViewController> mainMenu;

    // Callbacks
    void showLockScreen(Event event);
    void showBookReader(Event event);
    void returnHome(Event event);
    void changeRefreshMode(Event event);

    int requestedRefreshMode = -1;
};

#endif // OpenBookApplication_h
