#include "Widgets.h"

Label::Label(Rect rect, std::string text) : View(rect) {
    this->text = text;
}

void Label::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    View::draw(display, x, y);
    display->setTextColor(this->foregroundColor);
    display->setCursor(this->frame.origin.x + x, this->frame.origin.y + y);
    display->print(this->text.c_str());
}

void Label::setText(std::string text) {
    this->text = text;
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}