#include "BookListViewController.h"
#include "OpenBookEvents.h"
#include "bitmaps.h"
#include <sstream>
#include <iomanip>

void BookListViewController::viewWillAppear() {
    ViewController::viewWillAppear();

    // update books whenever the view appears
    std::vector<std::string> titles;
    uint32_t numBooks = OpenBookDatabase::sharedDatabase()->getNumberOfBooks();

    for (uint32_t i = 0 ; i < numBooks ; i++) {
        BookRecord record = OpenBookDatabase::sharedDatabase()->getBookRecord(i);
        std::string title = OpenBookDatabase::sharedDatabase()->getBookTitle(record);
        titles.push_back(title);
        this->books.push_back(record);
    }

    this->table->setItems(titles);
    this->updateBatteryIcon();
}

void BookListViewController::createView() {
    ViewController::createView();
    this->view = std::make_shared<View>(MakeRect(0, 0, 300, 400));
    std::shared_ptr<OpenBookLabel> titleLabel = std::make_shared<OpenBookLabel>(MakeRect(28, 8, 200, 16), "My Library");
    std::shared_ptr<BitmapView> shelfIcon = std::make_shared<BitmapView>(MakeRect(9, 9, 16, 16), ShelfIcon);
    titleLabel->setBold(true);
    this->batteryIcon = std::make_shared<BitmapView>(MakeRect(267, 9, 24, 9), BatteryIcon);
    this->usbIcon = std::make_shared<BitmapView>(MakeRect(267, 9, 24, 9), PlugIcon);
    this->voltageLabel = std::make_shared<Label>(MakeRect(228, 10, 36, 8), "0.00 V");
    this->batteryIcon->setHidden(true);
    this->usbIcon->setHidden(true);
    this->table = std::make_shared<OpenBookTable>(MakeRect(0, 32, 300, 400 - 32), 24, CellSelectionStyleIndicatorLeading);
    this->view->addSubview(this->table);
    this->view->addSubview(titleLabel);
    this->view->addSubview(shelfIcon);
    this->view->addSubview(this->batteryIcon);
    this->view->addSubview(this->usbIcon);
    this->view->addSubview(this->voltageLabel);

    this->view->setAction(std::bind(&BookListViewController::selectBook, this, std::placeholders::_1), BUTTON_TAP);
    this->view->setAction(std::bind(&BookListViewController::updateBatteryIcon, this, std::placeholders::_1), OPEN_BOOK_EVENT_POWER_CHANGED);
}

void BookListViewController::selectBook(Event event) {
    if (std::shared_ptr<Window>window = this->view->getWindow().lock()) {
        this->currentBook = this->books[event.userInfo];

        if (OpenBookDatabase::sharedDatabase()->bookIsPaginated(this->currentBook)) {
            this->generateEvent(OPEN_BOOK_EVENT_BOOK_SELECTED, (int32_t)&this->currentBook);
        } else {
            this->modal = std::make_shared<BorderedView>(MakeRect(20, 100, 300 - 20 * 2, 200));
            int16_t subviewWidth = this->modal->getFrame().size.width - 40;
            window->addSubview(this->modal);
            std::shared_ptr<OpenBookLabel> label = std::make_shared<OpenBookLabel>(MakeRect(20, 20, subviewWidth, 32), "This book is not paginated.\nPaginate it now?");
            this->modal->addSubview(label);
            std::shared_ptr<OpenBookButton> yes = std::make_shared<OpenBookButton>(MakeRect(20, 68, subviewWidth, 48), "Yes");
            yes->setAction(std::bind(&BookListViewController::paginate, this, std::placeholders::_1), BUTTON_TAP);
            this->modal->addSubview(yes);
            std::shared_ptr<OpenBookButton> no = std::make_shared<OpenBookButton>(MakeRect(20, 132, subviewWidth, 48), "No");
            no->setAction(std::bind(&BookListViewController::dismiss, this, std::placeholders::_1), BUTTON_TAP);
            this->modal->addSubview(no);
            this->modal->becomeFocused();
            this->generateEvent(OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE, OPEN_BOOK_DISPLAY_MODE_GRAYSCALE);
        }
    }
}

void BookListViewController::dismiss(Event event) {
    if (std::shared_ptr<Window>window = this->view->getWindow().lock()) {
        window->removeSubview(this->modal);
        this->modal.reset();
        this->currentBook = {0};
    }
}

void BookListViewController::paginate(Event event) {
    if (std::shared_ptr<Window>window = this->view->getWindow().lock()) {
        window->removeSubview(this->modal);
        this->modal.reset();
        OpenBookDatabase::sharedDatabase()->paginateBook(this->currentBook);
        this->generateEvent(OPEN_BOOK_EVENT_BOOK_SELECTED, (int32_t)&this->currentBook);
    }
}

void BookListViewController::updateBatteryIcon(Event event) {
    float systemVoltage = OpenBookDevice::sharedDevice()->getSystemVoltage();
    bool onBattery = systemVoltage < 4.5;
    this->batteryIcon->setHidden(!onBattery);
    this->usbIcon->setHidden(onBattery);
    std::stringstream ss;
    ss << std::right << std::setw(4) << std::setprecision(3) << systemVoltage << " V";
    this->voltageLabel->setText(ss.str());
}
