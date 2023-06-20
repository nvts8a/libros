#include "Application.h"

void Window::addSubview(std::shared_ptr<View> view) {
    view->setWindow(std::static_pointer_cast<Window>(this->shared_from_this()));
    View::addSubview(view);
    // when we add a new view hierarchy to the window, try to focus on its innermost view.
    this->becomeFocused();
}

bool Window::canBecomeFocused() {
    return true;
}

bool Window::needsDisplay() {
    return this->dirty;
}

void Window::setNeedsDisplay(bool needsDisplay) {
    if (needsDisplay) {
        this->dirtyRect = MakeRect(0, 0, this->frame.size.width, this->frame.size.height);
        this->dirty = true;
    } else {
        this->dirty = false;
    }
}

void Window::setNeedsDisplayInRect(Rect rect) {
    Rect finalRect;
    if (this->dirty) {
        finalRect = MakeRect(min(this->dirtyRect.origin.x, rect.origin.x), min(this->dirtyRect.origin.y, rect.origin.y), 0, 0);
        finalRect.size.width = max(this->dirtyRect.origin.x + this->dirtyRect.size.width, rect.origin.x + rect.size.width) - finalRect.origin.x;
        finalRect.size.height = max(this->dirtyRect.origin.y + this->dirtyRect.size.height, rect.origin.y + rect.size.height) - finalRect.origin.y;
    } else {
        finalRect = rect;
    }

    this->dirty = true;
    this->dirtyRect = finalRect;
}

Rect Window::getDirtyRect() {
    if (this->dirty) return this->dirtyRect;
    else return {0};
}

std::weak_ptr<View> Window::getFocusedView() {
    return this->focusedView;
}

std::weak_ptr<View> Window::getSuperview() {
    return std::weak_ptr<View>();
}

std::weak_ptr<Window> Window::getWindow() {
    return std::static_pointer_cast<Window, View>(this->shared_from_this());
}

void Window::setWindow(std::shared_ptr<Window> window) {
    // nothing to do here
}