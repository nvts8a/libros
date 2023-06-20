#include "Application.h"

ViewController::ViewController(std::shared_ptr<Application> application) {
    this->application = application;
}

void ViewController::viewWillAppear() {
    if (!this->view) {
        this->createView();
    }
}

void ViewController::viewDidDisappear() {
    this->destroyView();
}

void ViewController::generateEvent(int32_t eventType, int32_t userInfo) {
    if (!this->view) return;

    // unsure about this one: we generate an event and let it bubble up to the window,
    // where the application can listen for it. seems like wasted effort to get a message
    // from a view controller to the application.
    if (std::shared_ptr<Window> window = this->view->getWindow().lock()) {
        if (std::shared_ptr<Application> application = window->application.lock()) {
            application->generateEvent(eventType, userInfo);
        }
    }
}

void ViewController::createView() {
    if (this->view) {
        this->destroyView();
    }
    // subclasses must override to create view here
}

void ViewController::destroyView() {
    this->view.reset();
}