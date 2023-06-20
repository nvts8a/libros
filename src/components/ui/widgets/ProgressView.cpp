#include "Widgets.h"

void ProgressView::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    View::draw(display, x, y);
    display->fillRect(x + this->frame.origin.x, y + this->frame.origin.y, (int16_t)(this->frame.size.width * this->progress), this->frame.size.height, this->foregroundColor);
}

void ProgressView::setProgress(float value) {
    this->progress = value;
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}

float ProgressView::getProgress() {
    return this->progress;
}