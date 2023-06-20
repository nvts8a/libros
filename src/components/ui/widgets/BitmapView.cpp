#include "Widgets.h"

BitmapView::BitmapView(Rect rect, const unsigned char *bitmap) : View(rect) {
    this->bitmap = bitmap;
}

void BitmapView::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    View::draw(display, x, y);
    display->drawBitmap(this->frame.origin.x, this->frame.origin.y, this->bitmap, this->frame.size.width, this->frame.size.height, this->foregroundColor);
}