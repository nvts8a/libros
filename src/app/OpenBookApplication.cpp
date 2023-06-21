#include "OpenBookApplication.h"

#include "Logger.h"
#include "OpenBookDevice.h"
#include "OpenBookTasks.h"
#include "OpenBookEvents.h"
#include "LoadingViewController.h"
#include "BookReaderViewController.h"
#include "FatalErrorViewController.h"
#include "BabelSetupViewController.h"

void OpenBookApplication::setup() {
    // Tasks needed baseline, even for error scenarios
    bool ready = true;
    displayTask = std::make_shared<OpenBookDisplay>();
    addTask(this->displayTask);
    std::shared_ptr<Task> inputTask = std::make_shared<OpenBookRawButtonInput>();
    addTask(inputTask);

    // Setting splash page with loading messages
    loadingView = std::make_shared<LoadingViewController>(this->shared_from_this(), "Starting the SD Card service...");
    setRootViewController(this->loadingView);

    setLoadingMessage("Starting the SD Card service");
    if (OpenBookDevice::sharedDevice()->startSD()) {
        Logger::INFO("Started the SD Card service");
    } else {
        // There's no log here because if the SD card service fails it can't write anywhere
        requestedRefreshMode = OPEN_BOOK_DISPLAY_MODE_GRAYSCALE;
        std::shared_ptr<FatalErrorViewController> modal = std::make_shared<FatalErrorViewController>(this->shared_from_this(), "Please insert a MicroSD card.");
        setRootViewController(modal);
        ready = false;
    }

    if (ready) {
        setLoadingMessage("Starting Babel");
        if (!OpenBookDevice::sharedDevice()->startBabel()) {
            Logger::ERROR("Failed to start Babel.");
            this->requestedRefreshMode = OPEN_BOOK_DISPLAY_MODE_GRAYSCALE;
            std::shared_ptr<BabelSetupViewController> modal = std::make_shared<BabelSetupViewController>(this->shared_from_this());
            this->setRootViewController(modal);
            ready = false;
        }
    }

    if (ready) {
        setLoadingMessage("Starting the Database service");
        if (!OpenBookDatabase::sharedDatabase()->connect()) {
            Logger::ERROR("Failed to start Database service.");
            this->requestedRefreshMode = OPEN_BOOK_DISPLAY_MODE_GRAYSCALE;
            std::shared_ptr<FatalErrorViewController> modal = std::make_shared<FatalErrorViewController>(this->shared_from_this(), "Failed to process the\nOpenBook Database.");
            this->setRootViewController(modal);
            ready = false;
        }
    }

    if (ready) {
        setLoadingMessage("Loading new books");
        if (!OpenBookDatabase::sharedDatabase()->scanForNewBooks()) {
            // TODO: Display issue with scaning for books when we start returning falses
            Logger::ERROR("Failed to scan SD Card for changes.");
            this->requestedRefreshMode = OPEN_BOOK_DISPLAY_MODE_GRAYSCALE;
            ready = false;
        }
    }

    if (ready) {
        setLoadingMessage("Loading OpenBook");
        Logger::INFO("Setting up tasks for input, output, and lock screen");
        std::shared_ptr<Task> lockScreenTask = std::make_shared<OpenBookLockScreen>();
        this->addTask(lockScreenTask);
        std::shared_ptr<Task> powerTask = std::make_shared<OpenBookPowerMonitor>();
        this->addTask(powerTask);
        Logger::INFO("Setting up actions");
        this->window->setAction(std::bind(&OpenBookApplication::showLockScreen, this, std::placeholders::_1), BUTTON_LOCK);
        this->window->setAction(std::bind(&OpenBookApplication::showBookReader, this, std::placeholders::_1), OPEN_BOOK_EVENT_BOOK_SELECTED);
        this->window->setAction(std::bind(&OpenBookApplication::returnHome, this, std::placeholders::_1), OPEN_BOOK_EVENT_RETURN_HOME);
        this->window->setAction(std::bind(&OpenBookApplication::changeRefreshMode, this, std::placeholders::_1), OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE);

        setLoadingMessage("Loading Library");
        this->mainMenu = std::make_shared<BookListViewController>(this->shared_from_this());
        this->setRootViewController(this->mainMenu);
    }
}

void OpenBookApplication::setLoadingMessage(std::string message) {
    Logger::INFO(message);
    this->loadingView->setMessage(message);
    this->getWindow().get()->setNeedsDisplayInRect(MESSAGE_RECT);
    this->displayTask.get()->run(this->shared_from_this());
}

void OpenBookApplication::showLockScreen(Event event) {
    this->locked = true;
}

void OpenBookApplication::showBookReader(Event event) {
    BookRecord book = *(BookRecord *)event.userInfo;
    std::shared_ptr<BookReaderViewController> nextViewController = std::make_shared<BookReaderViewController>(this->shared_from_this(), book);
    this->setRootViewController(nextViewController);
}

void OpenBookApplication::returnHome(Event event) {
    this->setRootViewController(this->mainMenu);
}

void OpenBookApplication::changeRefreshMode(Event event) {
    this->requestedRefreshMode = event.userInfo;
}
