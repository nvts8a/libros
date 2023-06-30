#include "BookListViewController.h"
#include "OpenBookEvents.h"
#include "I18n.h"
#include "bitmaps.h"
#include <sstream>
#include <iomanip>

BookListViewController::BookListViewController(std::shared_ptr<Application> application) : ViewController(application) {
    database = OpenBookDatabase::sharedDatabase();
    numberOfBooks = database->getLibrarySize();
    numberOfPages = (this->numberOfBooks - 1) / LIBRARY_PAGE_SIZE;

    _updatePagination();
}

void BookListViewController::viewWillAppear() {
    ViewController::viewWillAppear();
    if (this->numberOfBooks > 0) this->_updateLibraryPage();
    this->updateBatteryIcon();
}

void BookListViewController::createView() {
    ViewController::createView();
    this->view = std::make_shared<View>(MakeRect(0, 0, 300, 400));
    std::shared_ptr<TypesetterLabel> titleLabel = std::make_shared<TypesetterLabel>(MakeRect(28, 8, 200, 16), I18n::get().bookList.myLibrary);
    std::shared_ptr<BitmapView> shelfIcon = std::make_shared<BitmapView>(MakeRect(9, 9, 16, 16), ShelfIcon);
    titleLabel->setBold(true);
    this->batteryIcon = std::make_shared<BitmapView>(MakeRect(267, 9, 24, 9), BatteryIcon);
    this->usbIcon = std::make_shared<BitmapView>(MakeRect(267, 9, 24, 9), PlugIcon);
    this->voltageLabel = std::make_shared<Label>(MakeRect(228, 10, 36, 8), "0.00 V");
    this->batteryIcon->setHidden(true);
    this->usbIcon->setHidden(true);
    this->table = std::make_shared<Table>(MakeRect(0, 32, 300, 400 - 32), 24, CellSelectionStyleIndicatorLeading);
    this->view->addSubview(this->table);
    this->view->addSubview(titleLabel);
    this->view->addSubview(shelfIcon);
    this->view->addSubview(this->batteryIcon);
    this->view->addSubview(this->usbIcon);
    this->view->addSubview(this->voltageLabel);

    this->pagination = std::make_shared<Label>(MakeRect(this->paginationLabelXPos, 400-40, 90, 8), this->paginationLabel);
    this->view->addSubview(this->pagination);

    if (this->numberOfBooks > 0) { // If there aren't any books, these actions will just err out
        this->view->setAction(std::bind(&BookListViewController::selectBook, this, std::placeholders::_1), BUTTON_TAP);
        this->view->setAction(std::bind(&BookListViewController::viewBookDetails, this, std::placeholders::_1), BUTTON_LEFT);
        this->view->setAction(std::bind(&BookListViewController::viewBookDetails, this, std::placeholders::_1), BUTTON_RIGHT);
        this->view->setAction(std::bind(&BookListViewController::previousPage, this, std::placeholders::_1), BUTTON_PREV);
        this->view->setAction(std::bind(&BookListViewController::nextPage, this, std::placeholders::_1), BUTTON_NEXT);
    }
    this->view->setAction(std::bind(&BookListViewController::updateBatteryIcon, this, std::placeholders::_1), OPEN_BOOK_EVENT_POWER_CHANGED);
}

void BookListViewController::selectBook(Event event) {
    if (std::shared_ptr<Window>window = view->getWindow().lock()) { 
        currentBook = (currentPage * LIBRARY_PAGE_SIZE) + event.userInfo;
        BookRecord currentBookRecord = OpenBookDatabase::sharedDatabase()->getLibraryBookRecord(currentBook);

        if (database->bookIsPaginated(currentBookRecord)) {
            generateEvent(OPEN_BOOK_EVENT_BOOK_SELECTED, (int32_t)&currentBookRecord);
        } else _createPaginationModal(window, database->getBookTitle(currentBookRecord));
    }
}

/**
 * Constructs a modal for asking the user if they would like to paginate an unpaginated book.
 * TODO: See if one day we can dynamically get the height of the label and just push the buttons down in a dynamic modal view
 * @param window This is the Window object, locked by the calling method so should be passed in
 *  as a dependency and not reobtained.
 * @param bookTitle The string title of the book, used to construct the pagination message. 
*/
void BookListViewController::_createPaginationModal(std::shared_ptr<Window> window, std::string bookTitle) {
    int16_t modalWidth   = 300 - (20 * 2);
    int16_t subviewWidth = modalWidth - 40;
    int16_t buttonWidth  = (subviewWidth / 2) - 5;
    
    std::shared_ptr<TypesetterLabel> label = std::make_shared<TypesetterLabel>(MakeRect(20, 20, subviewWidth, 64), bookTitle + I18n::get().bookList.pagination);
    label->setWordWrap(true);

    std::shared_ptr<TypesetterButton> yes = std::make_shared<TypesetterButton>(MakeRect(20, 116, buttonWidth, 32), I18n::get().common.yes);
    yes->setAction(std::bind(&BookListViewController::paginate, this, std::placeholders::_1), BUTTON_TAP);

    std::shared_ptr<TypesetterButton> no = std::make_shared<TypesetterButton>(MakeRect(20 + buttonWidth + 10, 116, buttonWidth, 32), I18n::get().common.no);
    no->setAction(std::bind(&BookListViewController::dismiss, this, std::placeholders::_1), BUTTON_TAP);
    
    modal = std::make_shared<BorderedView>(MakeRect(20, 100, modalWidth, 170));
    modal->setDirectionalAffinity(DirectionalAffinityHorizontal);
    modal->addSubview(label);
    modal->addSubview(yes);
    modal->addSubview(no);

    window->addSubview(modal);
    modal->becomeFocused();
    generateEvent(OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE, OPEN_BOOK_DISPLAY_MODE_GRAYSCALE);
}

/**
 * Constructs a modal for displaying the current book's description.
 * @param event Holds the current book number for the current page as .userInfo, though I'm not sure if it shouldbe named that
*/
void BookListViewController::viewBookDetails(Event event) {
    if (std::shared_ptr<Window>window = view->getWindow().lock()) {
        currentBook = (this->currentPage * LIBRARY_PAGE_SIZE) + event.userInfo;
        BookRecord  selectedBook =    OpenBookDatabase::sharedDatabase()->getLibraryBookRecord(currentBook);
        std::string bookTitle =       OpenBookDatabase::sharedDatabase()->getBookTitle(selectedBook);
        std::string bookAuthor =      OpenBookDatabase::sharedDatabase()->getBookAuthor(selectedBook);
        std::string bookDescription = OpenBookDatabase::sharedDatabase()->getBookDescription(selectedBook);

        modal = std::make_shared<BorderedView>(MakeRect(DEFAULT_PADDING, DEFAULT_PADDING, DEFAULT_WIDTH-(DEFAULT_PADDING*2), DEFAULT_HEIGHT-(DEFAULT_PADDING*2)));
        int16_t subviewWidth = modal->getFrame().size.width - (DEFAULT_PADDING*2);
        window->addSubview(modal);

        std::shared_ptr<TypesetterLabel> label = std::make_shared<TypesetterLabel>(MakeRect(DEFAULT_PADDING, DEFAULT_PADDING, subviewWidth, 32), bookTitle + "\n " + I18n::get().common.by + bookAuthor);
        label->setBold(true);
        modal->addSubview(label);

        int16_t closeWidth     = (I18n::get().common.close.length() * 8) + 16;
        int16_t closeXPosition = (modal->getFrame().size.width - closeWidth) / 2;
        int16_t closeYPosition = modal->getFrame().size.height - 32;
        std::shared_ptr<TypesetterButton> close = std::make_shared<TypesetterButton>(MakeRect(closeXPosition, closeYPosition, closeWidth, 24), I18n::get().common.close);
        close->setAction(std::bind(&BookListViewController::dismiss, this, std::placeholders::_1), BUTTON_TAP);
        modal->addSubview(close);

        int16_t descriptinYPosition = label.get()->getFrame().origin.y + label.get()->getFrame().size.height + DEFAULT_PADDING;
        int16_t descriptionHeight = modal->getFrame().size.height - descriptinYPosition - (modal->getFrame().size.height - closeYPosition) - DEFAULT_PADDING;
        std::shared_ptr<TypesetterLabel> descriptionLabel = std::make_shared<TypesetterLabel>(MakeRect(DEFAULT_PADDING, 64, subviewWidth, descriptionHeight), "  " + bookDescription);
        descriptionLabel->setWordWrap(true);
        modal->addSubview(descriptionLabel);

        modal->becomeFocused();
        generateEvent(OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE, OPEN_BOOK_DISPLAY_MODE_GRAYSCALE);
    }
}

void BookListViewController::dismiss(Event event) {
    if (std::shared_ptr<Window>window = view->getWindow().lock()) {
        window->removeSubview(modal);
        modal.reset();
    }
}

void BookListViewController::paginate(Event event) {
    if (std::shared_ptr<Window>window = this->view->getWindow().lock()) {
        window->removeSubview(this->modal);
        this->modal.reset();
        BookRecord currentBookRecord   = OpenBookDatabase::sharedDatabase()->getLibraryBookRecord(this->currentBook);
        BookRecord paginatedBookRecord = OpenBookDatabase::sharedDatabase()->paginateBook(currentBookRecord);
        OpenBookDatabase::sharedDatabase()->setLibraryBookRecord(this->currentBook, paginatedBookRecord);
        this->generateEvent(OPEN_BOOK_EVENT_BOOK_SELECTED, (int32_t)&paginatedBookRecord);
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

/**
 * Increments the library page if it hasn't hit it's max,
 *  then refreshes only if there was a page change
 * @param event UNUSED
*/
void BookListViewController::nextPage(Event event) {
    if (this->currentPage < this->numberOfPages) {
        this->currentPage++;
        this->_updateView();
    }
}

/**
 * Decrements the library page if it hasn't hit it's min,
 *  then refreshes only if there was a page change
 * @param event UNUSED
*/
void BookListViewController::previousPage(Event event) {
    if (this->currentPage > 0) {
        this->currentPage--;
        this->_updateView();
    }
}

/**
 * Sets the current pages text to the visible book text, with custom size for chapters.
 *  And updates the progress bar accordingly.
*/
void BookListViewController::_updateView() {
    // Set current library page table items
    this->_updateLibraryPage();

    // Set pagination label
    this->_updatePagination();
    this->pagination->setText(this->paginationLabel);
    this->pagination->setFrame(MakeRect(this->paginationLabelXPos, 400-40, 90, 8));
}

/**
 * Obtains the current page of BookRecords and constructs the string titles for them
 *  And sets those titles on the table
*/
void BookListViewController::_updateLibraryPage() {
    std::vector<std::string> titles;
    std::vector<BookRecord> currentPage = OpenBookDatabase::sharedDatabase()->getLibraryPage(this->currentPage);

    for (auto &bookRecord : currentPage) {
        std::string title = OpenBookDatabase::sharedDatabase()->getBookTitle(bookRecord);
        std::string author = OpenBookDatabase::sharedDatabase()->getBookAuthor(bookRecord);
        if (author != "") title += " by " + author;

        titles.push_back(title.substr(0, 35));
    }

    // Set current library page table items
    this->table->setItems(titles);
}

/**
 * If there are books, updates the pagination label for view creation or when you switch pages
*/
void BookListViewController::_updatePagination() {
    if (this->numberOfBooks > 0) {
        uint16_t pageStart = (LIBRARY_PAGE_SIZE * this->currentPage) + 1;
        uint16_t pageEnd = min((pageStart + LIBRARY_PAGE_SIZE - 1), this->numberOfBooks);
        std::string prefix = "   "; if (pageStart > 1)                 prefix = "<< ";
        std::string suffix = "   "; if (pageEnd < this->numberOfBooks) suffix = " >>";
        this->paginationLabel = prefix + std::to_string(pageStart) + '-' + std::to_string(pageEnd) + I18n::get().common.of + std::to_string(numberOfBooks) + suffix;
        this->paginationLabelXPos = 150 - ((this->paginationLabel.length() / 2) * 6);
    }
}