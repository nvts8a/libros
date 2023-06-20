#include "Widgets.h"

Button::Button(Rect rect, std::string text) : Control(rect) {
    this->text = text;
}

void Button::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        View::draw(display, x, y);
        display->setCursor(this->frame.origin.x + x + 4, this->frame.origin.y + y + this->frame.size.height / 2 - 4);
        if (this->focused) {
            display->fillRect(x + this->frame.origin.x, y + this->frame.origin.y, this->frame.size.width, this->frame.size.height, this->foregroundColor);
            display->setTextColor(this->backgroundColor);
            display->print(this->text.c_str());
        } else {
            display->drawRect(x + this->frame.origin.x, y + this->frame.origin.y, this->frame.size.width, this->frame.size.height, this->foregroundColor);
            display->setTextColor(this->foregroundColor);
            display->print(this->text.c_str());
        }
    }
}