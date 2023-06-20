#include "Widgets.h"

HatchedView::HatchedView(Rect rect, uint16_t color) : View(rect) {
    this->foregroundColor = color;
}

void HatchedView::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    for(int16_t i = x; i < x + this->frame.size.width; i++) {
        for(int16_t j = y; j < y + this->frame.size.height; j++) {
            if ((i + j) % 2) {
                display->drawPixel(i, j, this->foregroundColor);
            }
        }
    }
    View::draw(display, x, y);
}