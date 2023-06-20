#include "Widgets.h"

#include <algorithm>

/**
 * 
*/
Table::Table(Rect rect, int16_t cellHeight, CellSelectionStyle selectionStyle) : Control(rect) {
    this->selectionStyle = selectionStyle;
    this->cellHeight = cellHeight;
    this->cellsPerPage = rect.size.height / cellHeight;
}

/**
 * 
*/
void Table::setItems(std::vector<std::string> items) {
    this->items = items;
    this->updateCells();
}

/**
 * 
*/
void Table::updateCells() {
    this->subviews.clear();

    uint16_t end = this->startOffset + this->cellsPerPage;
    if (end > this->items.size()) end = this->items.size();
    std::vector<std::string>::iterator it;
    uint16_t i = 0;
    for(std::string text : this->items) {
        std::shared_ptr<TableCell> cell = std::make_shared<TableCell>(MakeRect(0, this->cellHeight * i++, this->frame.size.width, this->cellHeight), text, this->selectionStyle);
        this->addSubview(cell);
    }
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        window->setNeedsDisplay(true);
    }

    this->selectedIndex = 0;
    this->subviews[this->selectedIndex]->becomeFocused();
}

/**
 * 
*/
bool Table::becomeFocused() {
    if (this->selectedIndex >= 0 && this->selectedIndex < ((int32_t)this->subviews.size())) {
        return this->subviews[this->selectedIndex]->becomeFocused();
    }

    return Control::becomeFocused();
}

/**
 * 
*/
int32_t Table::getSelectedIndex() {
    return this->selectedIndex;
}

/**
 * 
*/
bool Table::handleEvent(Event event) {
    if (event.type == BUTTON_TAP || event.type == BUTTON_LEFT || event.type == BUTTON_RIGHT) {
        // if user selected an item in the table, add that user info to the event
        event.userInfo = this->selectedIndex;
        return View::handleEvent(event);
    } else if (event.type == BUTTON_UP || event.type == BUTTON_DOWN) {
        // if the user moved the cursor around, first let the view select what to focus...
        bool retval = View::handleEvent(event);
        // then see if one of our views is focused; if so, update our index.
        this->selectedIndex = -1;
        if (std::shared_ptr<Window> window = this->getWindow().lock()) {
            if (std::shared_ptr<View> focusedView = window->getFocusedView().lock()) {
                this->selectedIndex = std::distance(this->subviews.begin(), std::find(this->subviews.begin(), this->subviews.end(), focusedView));
            }
        }
        // and return the value of View::handleEvent
        return retval;
    }

    // all other event types, just pass it along.
    return View::handleEvent(event);
}