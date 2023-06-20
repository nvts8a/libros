#include "Widgets.h"

#include "OpenBookDevice.h"

TableCell::TableCell(Rect rect, std::string text, CellSelectionStyle selectionStyle) : Control(rect) {
    this->text = text;
    this->selectionStyle = selectionStyle;
}

void TableCell::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        View::draw(display, x, y);

        // TODO: This should get passed it to be agnostic of the device
        BabelTypesetterGFX *typesetter = OpenBookDevice::sharedDevice()->getTypesetter();
        typesetter->setBold(false);
        typesetter->setItalic(false);
        typesetter->setTextSize(1);
        typesetter->setParagraphSpacing(0);
        Point cursorPosition = {0};
        uint16_t textColor = this->foregroundColor;
        switch (this->selectionStyle) {
            case CellSelectionStyleInvert:
                if (this->isFocused()) {
                    textColor = this->backgroundColor;
                }
                cursorPosition = MakePoint(this->frame.origin.x + x + 8, this->frame.origin.y + y + this->frame.size.height / 2 - 8);
                break;
            case CellSelectionStyleIndicatorLeading:
                cursorPosition = MakePoint(this->frame.origin.x + x + 16, this->frame.origin.y + y + this->frame.size.height / 2 - 8);
                break;
            case CellSelectionStyleIndicatorTrailing:
                cursorPosition = MakePoint(this->frame.origin.x + x + 8, this->frame.origin.y + y + this->frame.size.height / 2 - 8);
                break;
            case CellSelectionStyleIndicatorAbove:
                cursorPosition = MakePoint(this->frame.origin.x + x, this->frame.origin.y + y + 5);
                break;
            case CellSelectionStyleIndicatorBelow:
                cursorPosition = MakePoint(this->frame.origin.x + x, this->frame.origin.y + y + this->frame.size.height - 16 - 5);
                break;
        }
        if (this->isFocused()) {
            Rect indicatorRect = this->_indicatorRect();
            typesetter->display->fillRect(x + indicatorRect.origin.x, y + indicatorRect.origin.y, indicatorRect.size.width, indicatorRect.size.height, this->foregroundColor);
        }
        typesetter->setTextColor(textColor);
        typesetter->setCursor(cursorPosition.x, cursorPosition.y);
        typesetter->print(this->text.c_str());
    }
}

void TableCell::didBecomeFocused() {
    if (this->superview.lock()) {
        if (std::shared_ptr<Window> window = this->getWindow().lock()) {
            std::shared_ptr<View> shared_this = this->shared_from_this();
            shared_this->setNeedsDisplayInRect(this->_indicatorRect());
        }
    }
}

void TableCell::didResignFocus() {
    if (this->superview.lock()) {
        if (std::shared_ptr<Window> window = this->getWindow().lock()) {
            std::shared_ptr<View> shared_this = this->shared_from_this();
            shared_this->setNeedsDisplayInRect(this->_indicatorRect());
        }
    }
}

Rect TableCell::_indicatorRect() {
    switch (this->selectionStyle) {
        case CellSelectionStyleInvert:
            return MakeRect(this->frame.origin.x, this->frame.origin.y, this->frame.size.width, this->frame.size.height);
        case CellSelectionStyleIndicatorLeading:
            return MakeRect(this->frame.origin.x, this->frame.origin.y, 8, this->frame.size.height);
        case CellSelectionStyleIndicatorTrailing:
            return MakeRect(this->frame.origin.x + this->frame.size.width - 8, this->frame.origin.y, 8, this->frame.size.height);
        case CellSelectionStyleIndicatorAbove:
            return MakeRect(this->frame.origin.x, this->frame.origin.y, this->frame.size.width, 4);
        case CellSelectionStyleIndicatorBelow:
            return MakeRect(this->frame.origin.x, this->frame.origin.y + this->frame.size.height - 4, this->frame.size.width, 4);
    }

    return {0};
}