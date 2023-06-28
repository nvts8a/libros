#include "Widgets.h"

#include "OpenBookDevice.h"

TypesetterButton::TypesetterButton(Rect rect, std::string text) : Button(rect, text) {}

void TypesetterButton::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        View::draw(display, x, y);
        BabelTypesetterGFX typesetter = *OpenBookDevice::sharedDevice()->getTypesetter();
        typesetter.setWordWrap(false);
        typesetter.setBold(false);
        typesetter.setItalic(false);
        typesetter.setTextSize(1);
        typesetter.setParagraphSpacing(0);
        typesetter.setCursor(this->frame.origin.x + x + 8, this->frame.origin.y + y + this->frame.size.height / 2 - 8);
        if (this->isFocused()) {
            typesetter.display->fillRect(x + this->frame.origin.x, y + this->frame.origin.y, this->frame.size.width, this->frame.size.height, this->foregroundColor);
            typesetter.setTextColor(this->backgroundColor);
            typesetter.print(this->text.c_str());
        } else {
            typesetter.display->drawRect(x + this->frame.origin.x, y + this->frame.origin.y, this->frame.size.width, this->frame.size.height, this->foregroundColor);
            typesetter.setTextColor(this->foregroundColor);
            typesetter.print(this->text.c_str());
        }
    }
}