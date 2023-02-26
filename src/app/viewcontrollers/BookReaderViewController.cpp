#include "BookReaderViewController.h"
#include "OpenBookEvents.h"
#include "Logger.h"
#include <sstream>

BookReaderViewController::BookReaderViewController(std::shared_ptr<Application> application, BookRecord book) : ViewController(application) {
    this->book = book;
    this->numPages = max(OpenBookDatabase::sharedDatabase()->numPages(this->book), 1);
    this->currentPage = OpenBookDatabase::sharedDatabase()->getCurrentPage(this->book);
    this->_getProgressLabelDetails();
}

void BookReaderViewController::viewWillAppear() {
    ViewController::viewWillAppear();
    this->_updateView();
}

void BookReaderViewController::createView() {
    ViewController::createView();
    this->view = std::make_shared<Control>(MakeRect(0, 0, 300, 400));

    this->bookText = std::make_shared<OpenBookLabel>(MakeRect(6, 2, 300 - 12, 800 - 26), "");
    this->bookText->setWordWrap(true);
    this->bookText->setLineSpacing(2);
    this->bookText->setParagraphSpacing(8);
    this->view->addSubview(this->bookText);

    this->progressView = std::make_shared<ProgressView>(MakeRect(0, PROGRESS_BAR_YPOS, 300, 4));
    this->view->addSubview(this->progressView);
    this->progressLabel = std::make_shared<Label>(MakeRect(this->progressLabelXPos, PROGRESS_LABEL_YPOS, 90, 8), "");
    this->view->addSubview(this->progressLabel);

    this->view->setAction(std::bind(&BookReaderViewController::returnHome, this, std::placeholders::_1), BUTTON_TAP);
    this->view->setAction(std::bind(&BookReaderViewController::saveProgress, this, std::placeholders::_1), BUTTON_LOCK);
    this->view->setAction(std::bind(&BookReaderViewController::previousPage, this, std::placeholders::_1), BUTTON_PREV);
    this->view->setAction(std::bind(&BookReaderViewController::nextPage, this, std::placeholders::_1), BUTTON_NEXT);
    this->view->setAction(std::bind(&BookReaderViewController::showMenu, this, std::placeholders::_1), BUTTON_UP);
}

/**
 * Increments the currents page if it hasn't hit it's max,
 *  then refreshes only if there was a page change
 * @param event UNUSED
*/
void BookReaderViewController::nextPage(Event event) {
    if (this->currentPage < this->numPages-1) {
        this->currentPage++;
        this->_updateView();
    }
}

/**
 * Decrements the currents page if it hasn't hit it's min,
 *  then refreshes only if there was a page change
 * @param event UNUSED
*/
void BookReaderViewController::previousPage(Event event) {
    if (this->currentPage > 0) {
        this->currentPage--;
        this->_updateView();
    }
}

void BookReaderViewController::returnHome(Event event) {
    this->saveProgress(event);
    this->generateEvent(OPEN_BOOK_EVENT_RETURN_HOME);
}

void BookReaderViewController::saveProgress(Event event) {
    OpenBookDatabase::sharedDatabase()->setCurrentPage(this->book, this->currentPage);
}

void BookReaderViewController::showMenu(Event event) {
    this->modal = std::make_shared<Control>(MakeRect(50, 200, 200, 200));

    std::shared_ptr<BorderedView> modalFrame = std::make_shared<BorderedView>(MakeRect(0, 0, 200, 201));
    modalFrame->setForegroundColor(EPD_BLACK);
    modalFrame->setBackgroundColor(EPD_WHITE);
    this->modal->addSubview(modalFrame);

    std::string bookTitle = OpenBookDatabase::sharedDatabase()->getBookTitle(this->book);
    std::string bookAuthor = OpenBookDatabase::sharedDatabase()->getBookAuthor(this->book);
    std::shared_ptr<OpenBookLabel> titleLabel = std::make_shared<OpenBookLabel>(MakeRect(4, 4, 200 - 8, 128), bookTitle.append(" by ").append(bookAuthor));
    titleLabel->setBold(true);
    titleLabel->setWordWrap(true);
    this->modal->addSubview(titleLabel);

    std::stringstream ss;
    ss << "Go to page " << this->currentPage + 1;
    this->gotoPageLabel = std::make_shared<OpenBookLabel>(MakeRect(4, 200 - 48, 200 - 8, 16), ss.str());
    this->gotoPageLabel->setItalic(true);
    this->modal->addSubview(this->gotoPageLabel);

    this->modalSlider = std::make_shared<ProgressView>(MakeRect(4, 200 - 24, 200 - 8, 16));
    this->modalSlider->setProgress(this->progressView->getProgress());
    this->modal->addSubview(this->modalSlider);

    this->view->addSubview(this->modal);

    this->modal->setAction(std::bind(&BookReaderViewController::handleModal, this, std::placeholders::_1), BUTTON_LEFT);
    this->modal->setAction(std::bind(&BookReaderViewController::handleModal, this, std::placeholders::_1), BUTTON_RIGHT);
    this->modal->setAction(std::bind(&BookReaderViewController::handleModal, this, std::placeholders::_1), BUTTON_TAP);

    this->modal->becomeFocused();
    this->generateEvent(OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE, OPEN_BOOK_DISPLAY_MODE_GRAYSCALE);
}

void BookReaderViewController::handleModal(Event event) {
    float percentComplete;
    std::stringstream ss;
    switch (event.type) {
        case BUTTON_LEFT:
            this->currentPage = max(this->currentPage - 10, 0);
            percentComplete = (float)(this->currentPage) / (float)(this->numPages);
            ss << "Go to page " << this->currentPage + 1;
            this->modalSlider->setProgress(percentComplete);
            this->gotoPageLabel->setText(ss.str());
            this->generateEvent(OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE, OPEN_BOOK_DISPLAY_MODE_PARTIAL);
            break;
        case BUTTON_RIGHT:
            this->currentPage = min(this->currentPage + 10, this->numPages);
            percentComplete = (float)(this->currentPage) / (float)(this->numPages);
            ss << "Go to page " << this->currentPage + 1;
            this->modalSlider->setProgress(percentComplete);
            this->gotoPageLabel->setText(ss.str());
            this->generateEvent(OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE, OPEN_BOOK_DISPLAY_MODE_PARTIAL);
            break;
        case BUTTON_TAP:
            this->view->removeSubview(this->modal);
            this->modal = NULL;
            this->modalSlider = NULL;
            this->gotoPageLabel = NULL;
            this->generateEvent(OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE, OPEN_BOOK_DISPLAY_MODE_QUICK);
            _updateView();
            break;
    }
}

/**
 * Sets the current pages text to the visible book text, with custom size for chapters.
 *  And updates the progress bar accordingly.
*/
void BookReaderViewController::_updateView() {
    // Set the page text
    std::string text = OpenBookDatabase::sharedDatabase()->getTextForPage(this->book, this->currentPage);
    this->bookText->setText(text.c_str());

    // Set the text size
    if (text[0] == OpenBookDatabase_h::CHAPTER_MARK) this->bookText->setTextSize(2);
    else this->bookText->setTextSize(1);

    // Set the progress bar and label
    this->_getProgressLabelDetails();
    this->progressView->setProgress(this->progressPercentage);
    this->progressLabel->setText(this->progressText);
    this->progressLabel->setForegroundColor(EPD_BLACK);
    this->progressLabel->setFrame(MakeRect(this->progressLabelXPos, PROGRESS_LABEL_YPOS, 90, 8));

    this->generateEvent(OPEN_BOOK_EVENT_REQUEST_REFRESH_MODE, OPEN_BOOK_DISPLAY_MODE_QUICK);
}

/**
 * Uses object variables of current page and the number of pages to perform calculations and
 *  data massaging for reader progress bar data, including percentage, label text, and label position.
*/
void BookReaderViewController::_getProgressLabelDetails() {
    this->progressPercentage = (float)(this->currentPage + 1) / (float)(this->numPages);
    this->progressText = std::to_string(this->currentPage + 1) + '/' + std::to_string(this->numPages);
    int16_t calculatedPosition = (this->progressPercentage * 300) - (this->progressText.length() * 6);
    int16_t maxPosition = 300 - this->progressText.length();
    this->progressLabelXPos = min(max(0, calculatedPosition), maxPosition);

    Logger::l()->debug("Updated Progress Label details- percentage: " + std::to_string(this->progressPercentage));
    Logger::l()->debug("Updated Progress Label details- text: " + this->progressText);
    Logger::l()->debug("Updated Progress Label details- label x-position: " + std::to_string(this->progressLabelXPos));
}