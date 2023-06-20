#include "OpenBookDisplay.h"
#include "OpenBookDevice.h"
#include "OpenBookApplication.h"
#include "bitmaps.h"

OpenBookDisplay::OpenBookDisplay() {
    OpenBookDevice::sharedDevice()->startDisplay();
    display = OpenBookDevice::sharedDevice()->getDisplay();
    display->setDisplayMode(OPEN_BOOK_DISPLAY_MODE_QUICK);
    splash();
}

bool OpenBookDisplay::run(std::shared_ptr<Application> application) {
    OpenBookApplication *myApp = (OpenBookApplication *)application.get();

    std::shared_ptr<Window> window = application->getWindow();
    if (window->needsDisplay()) {
        display->clearBuffer();
        window->draw(display, 0, 0);

        Rect dirtyRect = window->getDirtyRect();

        if (myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_DEFAULT || myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_QUICK || myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_GRAYSCALE) {
            display->setDisplayMode((OpenBookDisplayMode)myApp->requestedRefreshMode);
            myApp->requestedRefreshMode = -1;
            display->display();
        } else if (RectsEqual(dirtyRect, window->getFrame())) {
            display->setDisplayMode(OPEN_BOOK_DISPLAY_MODE_QUICK);
            display->display();
        } else {
            if (myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_FASTPARTIAL || myApp->requestedRefreshMode == OPEN_BOOK_DISPLAY_MODE_PARTIAL) {
                display->setDisplayMode((OpenBookDisplayMode)myApp->requestedRefreshMode);
                myApp->requestedRefreshMode = -1;
            } else {
                display->setDisplayMode(OPEN_BOOK_DISPLAY_MODE_PARTIAL);
            }
            display->displayPartial(dirtyRect.origin.x, dirtyRect.origin.y, dirtyRect.size.width, dirtyRect.size.height);
        }
        window->setNeedsDisplay(false);
    }

    return false;
}

void OpenBookDisplay::splash() {
    display->clearBuffer();
    display->drawBitmap(0, 0, OpenBookSplash, 300, 400, EPD_BLACK);
    display->display();
}