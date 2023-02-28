#include "OpenBookApplication.h"
#include "Logger.h"
#include "OpenBookDevice.h"
#include "OpenBookTasks.h"
#include "OpenBookEvents.h"
#include "BookReaderViewController.h"
#include "FatalErrorViewController.h"
#include "BabelSetupViewController.h"

void OpenBookApplication::setup() {
    bool ready = true;
    if (OpenBookDevice::sharedDevice()->startSD()) {
        Logger::INFO("Started the SD Card service.");
        Logger::INFO("Starting the OpenBook Application...");
    } else {
        this->requestedRefreshMode = OPEN_BOOK_DISPLAY_MODE_GRAYSCALE;
        std::shared_ptr<FatalErrorViewController> modal = std::make_shared<FatalErrorViewController>(this->shared_from_this(), "Please insert a MicroSD card.");
        this->setRootViewController(modal);
        ready = false;
    }

    if (ready) {
        Logger::INFO("Starting Babel...");
        if (!OpenBookDevice::sharedDevice()->startBabel()) {
            Logger::ERROR("Failed to start Babel.");
            this->requestedRefreshMode = OPEN_BOOK_DISPLAY_MODE_GRAYSCALE;
            std::shared_ptr<BabelSetupViewController> modal = std::make_shared<BabelSetupViewController>(this->shared_from_this());
            this->setRootViewController(modal);
            ready = false;
        }
    }

    if (ready) {
        Logger::INFO("Starting the Database service...");
        if (!OpenBookDatabase::sharedDatabase()->connect()) {
            Logger::ERROR("Failed to start Database service.");
            this->requestedRefreshMode = OPEN_BOOK_DISPLAY_MODE_GRAYSCALE;
            std::shared_ptr<FatalErrorViewController> modal = std::make_shared<FatalErrorViewController>(this->shared_from_this(), "Failed to process the OpenBook Database.");
            this->setRootViewController(modal);
            ready = false;
        }
    }

    if (ready) {
        Logger::INFO("Scanning the SD Card for changes...");
        if (!OpenBookDatabase::sharedDatabase()->scanForNewBooks()) {
            // TODO: Display issue with scaning for books when we start returning falses
            Logger::ERROR("Failed to scan SD Card for changes.");
            this->requestedRefreshMode = OPEN_BOOK_DISPLAY_MODE_GRAYSCALE;
            ready = false;
        }
    }

    if (ready) {
        Logger::INFO("OpenBook Application successfully started. Constructing views.");
        Logger::INFO("Setting up tasks for input, output, and lock screen...");
        std::shared_ptr<Task> lockScreenTask = std::make_shared<OpenBookLockScreen>();
        this->addTask(lockScreenTask);
        std::shared_ptr<Task> inputTask = std::make_shared<OpenBookRawButtonInput>();
        this->addTask(inputTask);
        std::shared_ptr<Task> powerTask = std::make_shared<OpenBookPowerMonitor>();
        this->addTask(powerTask);
        std::shared_ptr<Task> displayTask = std::make_shared<OpenBookDisplay>();
        this->addTask(displayTask);

        Logger::INFO("Setting up BookListView...");
        this->mainMenu = std::make_shared<BookListViewController>(this->shared_from_this());
        this->setRootViewController(this->mainMenu);

        Logger::INFO("Setting up actions..");
        this->window->setAction(std::bind(&OpenBookApplication::showLockScreen, this, std::placeholders::_1), BUTTON_LOCK);
        this->window->setAction(std::bind(&OpenBookApplication::showBookReader, this, std::placeholders::_1), OPEN_BOOK_EVENT_BOOK_SELECTED);
        this->window->setAction(std::bind(&OpenBookApplication::returnHome, this, std::placeholders::_1), OPEN_BOOK_EVENT_RETURN_HOME);
        this->window->setAction(std::bind(&OpenBookApplication::changeRefreshMode, this, std::placeholders::_1), OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE);
    }
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
