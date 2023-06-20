#include "OpenBookLockScreen.h"
#include "OpenBookApplication.h"

bool OpenBookLockScreen::run(std::shared_ptr<Application> application) {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    OpenBookApplication *myApp = (OpenBookApplication *)application.get();
    
    if (myApp->locked) {
        std::shared_ptr<Window> window = application->getWindow();
        OpenBook_IL0398 *display = device->getDisplay();
        std::shared_ptr<BorderedView> lockModal = std::make_shared<BorderedView>(MakeRect(-1, 400-32, 302, 33));
        std::shared_ptr<TypesetterLabel> lockLabel = std::make_shared<TypesetterLabel>(MakeRect(6, 8, 300 - 16, 16), "Slide the power switch to continue");
        std::shared_ptr<TypesetterLabel> arrowLabel = std::make_shared<TypesetterLabel>(MakeRect(300-18, 10, 16, 16), "➜");

        lockModal->addSubview(lockLabel);
        lockModal->addSubview(arrowLabel);

        lockModal->setOpaque(true);
        lockModal->setBackgroundColor(EPD_WHITE);
        window->addSubview(lockModal);

        display->clearBuffer();
        window->draw(display, 0, 0);

        // this is pretty cool, if accidental: EPD_DARK renders as white with the
        // default or quick LUT, but dark in grayscale. so if we render this in
        // grayscale, it makes the white areas of the main screen appear dimmed,
        // as though we put a black layer with partial opacity.
        display->setDisplayMode(OPEN_BOOK_DISPLAY_MODE_GRAYSCALE);
        display->display();
        device->lockDevice(); // we remain here in dormant mode until the lock button is pressed.
        // at this time, the open book hardware resets when leaving low power mode, so the below code never runs.
        window->removeSubview(lockModal);
        window->setNeedsDisplay(true);
        myApp->locked = false;
    }

    return false;
}