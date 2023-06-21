#include "OpenBookDisplay.h"
#include "OpenBookDevice.h"
#include "OpenBookApplication.h"
#include "bitmaps.h"

OpenBookDisplay::OpenBookDisplay() {
    OpenBookDevice::sharedDevice()->startDisplay();
    displayDevice = OpenBookDevice::sharedDevice()->getDisplay();
    displayDevice->setDisplayMode(OPEN_BOOK_DISPLAY_MODE_QUICK);
}

bool OpenBookDisplay::run(std::shared_ptr<Application> application) {
    OpenBookApplication *myApp = (OpenBookApplication *)application.get();
    std::shared_ptr<Window> window = application->getWindow();

    if (window->needsDisplay()) {
        this->displayDevice->clearBuffer();
        window->draw(displayDevice, 0, 0);

        Rect dirtyRect = window->getDirtyRect();

        if (myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_DEFAULT || myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_QUICK || myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_GRAYSCALE) {
            this->display((OpenBookDisplayMode)myApp->requestedRefreshMode);
            myApp->requestedRefreshMode = -1;
        } else if (RectsEqual(dirtyRect, window->getFrame())) {
            this->display();
        } else {
            if (myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_FASTPARTIAL || myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_PARTIAL) {
                this->displayPartial((OpenBookDisplayMode)myApp->requestedRefreshMode, dirtyRect);
                myApp->requestedRefreshMode = -1;
            } else {
                this->displayPartial(dirtyRect);
            }
        }
        window->setNeedsDisplay(false);
    }

    return false;
}

void OpenBookDisplay::display() {
    this->display(OPEN_BOOK_DISPLAY_MODE_QUICK);
}

void OpenBookDisplay::display(OpenBookDisplayMode displayMode) {
    displayDevice->setDisplayMode(displayMode);
    displayDevice->display();
}

void OpenBookDisplay::displayPartial(Rect dirtyRect) {
    this->displayPartial(OPEN_BOOK_DISPLAY_MODE_PARTIAL, dirtyRect);
}

void OpenBookDisplay::displayPartial(OpenBookDisplayMode displayMode, Rect dirtyRect) {
    displayDevice->setDisplayMode(displayMode);
    displayDevice->displayPartial(dirtyRect.origin.x, dirtyRect.origin.y, dirtyRect.size.width, dirtyRect.size.height);
}