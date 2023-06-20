#include "Widgets.h"

BorderedView::BorderedView(Rect rect) : View(rect) {
    this->opaque = true;
}

void BorderedView::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    View::draw(display, x, y);
    display->drawRect(x + this->frame.origin.x, y + this->frame.origin.y, this->frame.size.width, this->frame.size.height, this->foregroundColor);
}