#include "Application.h"

#include <algorithm>

View::View(Rect rect) {
    // Serial.print("Creating view ");
    // Serial.println((int32_t) this);
    this->frame = rect;
    this->window.reset();
    this->superview.reset();
}

View::~View() {
    // Serial.print("Destroying view ");
    // Serial.println((int32_t) this);
}

void View::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    // Serial.print("Drawing view ");
    // Serial.println((int32_t) this);
    if (this->opaque || this->backgroundColor) {
        display->fillRect(x + this->frame.origin.x, y + this->frame.origin.y, this->frame.size.width, this->frame.size.height, this->backgroundColor);
    }
    for(std::shared_ptr<View> view : this->subviews) {
        if (!view->hidden) view->draw(display, this->frame.origin.x, this->frame.origin.y);
    }
}

void View::addSubview(std::shared_ptr<View> view) {
    view->superview = this->shared_from_this();
    this->subviews.push_back(view);
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        view->setWindow(window);
        window->setNeedsDisplay(true);
    }
}

void View::removeSubview(std::shared_ptr<View> view) {
    if (view->isFocused()) {
        view->resignFocus();
    }
    view->superview.reset();
    view->window.reset();
    int index = std::distance(this->subviews.begin(), std::find(this->subviews.begin(), this->subviews.end(), view));
    this->subviews.erase(this->subviews.begin() + index);    
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        // FIXME: We should only refocus if we know the focused view was removed.
        window->becomeFocused();
        window->setNeedsDisplay(true);
    }
}

bool View::isFocused() {
    return this->focused;
}

bool View::canBecomeFocused() {
    return false;
}

bool View::becomeFocused() {
    for(std::shared_ptr<View> subview : this->subviews) {
        if (subview->becomeFocused()) {
            return true;
        }
    }

    if (this->canBecomeFocused()) {
        // when there are no focusable subviews, and we can become
        // focused, become focused ourselves.
        // note that Window can always become focused, so this
        // block is guaranteed to execute when we reach the window.
        if (std::shared_ptr<Window> window = this->getWindow().lock()) {
            std::shared_ptr<View> oldResponder = window->getFocusedView().lock();
            if (oldResponder != NULL) {
                oldResponder->willResignFocus();
                oldResponder->focused = false;
                window->focusedView.reset();
                oldResponder->didResignFocus();
            }
            this->willBecomeFocused();
            this->focused = true;
            window->focusedView = this->shared_from_this();
            this->didBecomeFocused();
        }

        return true;
    }

    return false;
}

void View::resignFocus() {
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        if (std::shared_ptr<View> superview = this->superview.lock()) {
            superview->becomeFocused();
        }
    }
}

void View::movedToWindow() {
    // nothing to do here
}

void View::willBecomeFocused() {
    // nothing to do here
}

void View::didBecomeFocused() {
    if (this->superview.lock()) {
        if (std::shared_ptr<Window> window = this->getWindow().lock()) {
            std::shared_ptr<View> shared_this = this->shared_from_this();
            shared_this->setNeedsDisplayInRect(this->frame);
        }
    }
}

void View::willResignFocus() {
    // nothing to do here
}

void View::didResignFocus() {
    if (this->superview.lock()) {
        if (std::shared_ptr<Window> window = this->getWindow().lock()) {
            std::shared_ptr<View> shared_this = this->shared_from_this();
            shared_this->setNeedsDisplayInRect(this->frame);
        }
    }
}

bool View::handleEvent(Event event) {
    std::shared_ptr<View> focusedView = NULL;
    std::shared_ptr<Window> window = NULL;
    if (window = this->getWindow().lock()) {
        focusedView = window->getFocusedView().lock();
    } else {
        focusedView = this->shared_from_this();
        if (focusedView == NULL) return false;
        window = std::static_pointer_cast<Window, View>(focusedView);
    }

    if (this->actions.count(event.type)) {
        if (std::shared_ptr<Application> application = window->application.lock()) {
            this->actions[event.type](event);
        }
    } else if (event.type < BUTTON_TAP) {
        uint32_t index = std::distance(this->subviews.begin(), std::find(this->subviews.begin(), this->subviews.end(), focusedView));
        if (this->affinity == DirectionalAffinityVertical) {
            switch (event.type) {
                case BUTTON_UP:
                    while (index > 0) {
                        if (this->subviews[index - 1]->canBecomeFocused()) this->subviews[index - 1]->becomeFocused();
                        else index--;
                        return true;
                    }
                    break;
                case BUTTON_DOWN:
                    while ((index + 1) < this->subviews.size()) {
                        if (this->subviews[index + 1]->canBecomeFocused()) this->subviews[index + 1]->becomeFocused();
                        else index--;
                        return true;
                    }
                    break;
                default:
                    break;
            }
        } else if (this->affinity == DirectionalAffinityHorizontal) {
            switch (event.type) {
                case BUTTON_LEFT:
                    while (index > 0) {
                        if (this->subviews[index - 1]->canBecomeFocused()) this->subviews[index - 1]->becomeFocused();
                        return true;
                    }
                    break;
                case BUTTON_RIGHT:
                    while ((index + 1) < this->subviews.size()) {
                        if (this->subviews[index + 1]->canBecomeFocused()) this->subviews[index + 1]->becomeFocused();
                        return true;
                    }
                    break;
                default:
                    break;
            }
        }
    }
    if (std::shared_ptr<View> superview = this->superview.lock()) {
        superview->handleEvent(event);
    }

    return false;
}

void View::setAction(const Action &action, int32_t type) {
    this->actions[type] = action;
}

void View::removeAction(int32_t type) {
    // TODO: remove the action
}

std::weak_ptr<View> View::getSuperview() {
    return this->superview;
}

std::weak_ptr<Window> View::getWindow() {
    return this->window;
}

void View::setWindow(std::shared_ptr<Window>window) {
    this->window = window;
    for(std::shared_ptr<View> subview : this->subviews) {
        subview->setWindow(window);
    }
}

Rect View::getFrame() {
    return this->frame;
}

void View::setFrame(Rect frame) {
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        Rect dirtyRect = MakeRect(min(this->frame.origin.x, frame.origin.x), min(this->frame.origin.y, frame.origin.y), 0, 0);
        dirtyRect.size.width = max(this->frame.origin.x + this->frame.size.width, frame.origin.x + frame.size.width) - dirtyRect.origin.x;
        dirtyRect.size.height = max(this->frame.origin.y + this->frame.size.height, frame.origin.y + frame.size.height) - dirtyRect.origin.y;
        this->frame = frame;
        this->setNeedsDisplayInRect(dirtyRect);
    }
}

bool View::isOpaque() {
    return this->opaque;
}

void View::setOpaque(bool value) {
    if (this-> opaque == value) return;

    this->opaque = value;
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}

bool View::isHidden() {
    return this->hidden;
}

void View::setHidden(bool value) {
    if (this-> hidden == value) return;

    this->hidden = value;
    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        this->setNeedsDisplayInRect(this->frame);
    }
}

int32_t View::getTag() {
    return this->tag;
}

void View::setTag(int32_t value) {
    this->tag = value;
}

uint16_t View::getBackgroundColor() {
    return this->backgroundColor;
}

void View::setBackgroundColor(uint16_t value) {
    this->backgroundColor = value;
}

uint16_t View::getForegroundColor() {
    return this->foregroundColor;
}

void View::setForegroundColor(uint16_t value) {
    this->foregroundColor = value;
}

uint16_t View::getDirectionalAffinity() {
    return this->affinity;
}

void View::setDirectionalAffinity(DirectionalAffinity value) {
    this->affinity = value;
}

void View::setNeedsDisplayInRect(Rect rect) {
    std::shared_ptr<View> shared_this = this->shared_from_this();
    std::shared_ptr<View> superview(shared_this);
    while(superview = superview->superview.lock()) {
        rect.origin.x += superview->frame.origin.x;
        rect.origin.y += superview->frame.origin.y;
    }

    if (std::shared_ptr<Window> window = this->getWindow().lock()) {
        window->setNeedsDisplayInRect(rect);
    }
}
