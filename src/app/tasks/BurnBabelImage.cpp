#include "BurnBabelImage.h"

#include "OpenBookDevice.h"
#include "OpenBookDatabase.h"
#include "BookListViewController.h"
#include "OpenBookEvents.h"

bool BurnBabelImage::run(std::shared_ptr<Application> application) {
#ifdef ARDUINO_ARCH_RP2040
    static Adafruit_FlashTransport_SPI transport(1, SPI0);
    static Adafruit_SPIFlash flash(&transport);
    if (this->page == -1) {
        flash.begin();
        this->numPages = flash.numPages();
        flash.eraseChip();
        this->babelFile = OpenBookDevice::sharedDevice()->openFile("babel.bin");
    } else if (this->page < this->numPages) {
        uint8_t buffer[256];
        int16_t bytesRead = this->babelFile.read(buffer, 256);
        int16_t bytesWritten = flash.writeBuffer(this->page * 256, buffer, bytesRead);
        if (bytesRead != bytesWritten) {
            // TODO: Recover from this state?
            Serial.println("Flash error?");
        }
    } else {
        this->babelFile.close();
        OpenBookDevice::sharedDevice()->startBabel();
        OpenBookDatabase::sharedDatabase()->connect();
        OpenBookDatabase::sharedDatabase()->scanForNewBooks();

        std::shared_ptr<BookListViewController> mainViewController = std::make_shared<BookListViewController>(application);
        application->generateEvent(OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE, OPEN_BOOK_DISPLAY_MODE_DEFAULT);
        application->setRootViewController(mainViewController);

        return true;
    }
    int32_t progress = (int32_t)(100 * (float)this->page / (float)this->numPages);
    if (progress != this->lastUpdate) {
        this->lastUpdate = progress;
        application->generateEvent(OPEN_BOOK_EVENT_PROGRESS, progress);
    }
    this->page++;

    return false;
#else
    return true;
#endif
}
