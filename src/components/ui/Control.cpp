#include "Application.h"

Control::Control(Rect rect) : View(rect) {}

bool Control::isEnabled() {
    return this->enabled;
}

void Control::setEnabled(bool value) {
    this->enabled = value;
}

bool Control::canBecomeFocused() {
    return this->enabled;
}

Window::Window(Size size) : View(MakeRect(0, 0, size.width, size.height)) {
    this->setNeedsDisplay(true);
}