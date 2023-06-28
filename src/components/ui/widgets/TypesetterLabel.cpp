#include "Widgets.h"

#include "OpenBookDevice.h"

TypesetterLabel::TypesetterLabel(Rect rect, std::string text) : Label(rect, text) {}

void TypesetterLabel::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    View::draw(display, x, y);
    BabelTypesetterGFX typesetter = *OpenBookDevice::sharedDevice()->getTypesetter();
    typesetter.setLayoutArea(this->frame.origin.x + x, this->frame.origin.y + y, this->frame.size.width, this->frame.size.height);
    typesetter.setTextColor(this->foregroundColor);
    typesetter.setWordWrap(this->wrap);
    typesetter.setBold(this->bold);
    typesetter.setItalic(this->italic);
    typesetter.setTextSize(this->textSize);
    typesetter.setLineSpacing(this->lineSpacing);
    typesetter.setParagraphSpacing(this->paragraphSpacing);
    typesetter.print(this->text.c_str());
}

void TypesetterLabel::setWordWrap(bool value) {
    this->wrap = value;
    // TODO: setNeedsDisplay should be a method on View
    if (auto window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}

void TypesetterLabel::setBold(bool value) {
    this->bold = value;
    // TODO: setNeedsDisplay should be a method on View
    if (auto window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}

void TypesetterLabel::setItalic(bool value) {
    this->italic = value;
    if (auto window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}

void TypesetterLabel::setTextSize(uint16_t value) {
    this->textSize = value;
    if (auto window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}

void TypesetterLabel::setLineSpacing(uint16_t value) {
    this->lineSpacing = value;
    if (auto window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}

void TypesetterLabel::setParagraphSpacing(uint16_t value) {
    this->paragraphSpacing = value;
    if (auto window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}