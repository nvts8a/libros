#include "Application.h"

#include <algorithm>

Application::Application(int16_t width, int16_t height) {
    this->window = std::make_shared<Window>(MakeSize(width, height));
}

void Application::addTask(std::shared_ptr<Task> task) {
    this->tasks.push_back(task);
}

void Application::run() {
    this->setup();
    std::shared_ptr<Application> application = this->shared_from_this();
    this->window->application = application;
    this->window->becomeFocused();
    this->window->setNeedsDisplay(true);
    while(true) {
        for(std::shared_ptr<Task> task : this->tasks) {
            if (task->run(application)) {
                int index = std::distance(this->tasks.begin(), std::find(this->tasks.begin(), this->tasks.end(), task));
                this->tasks.erase(this->tasks.begin() + index);
            }
        }
    }
}

void Application::generateEvent(int32_t eventType, int32_t userInfo) {
    Event event;
    event.type = eventType;
    event.userInfo = userInfo;
    if (std::shared_ptr<View> focusedView = this->window->focusedView.lock()) {
        focusedView->handleEvent(event);
    }
}

std::shared_ptr<Window> Application::getWindow() {
    return this->window;
}

void Application::setRootViewController(std::shared_ptr<ViewController> viewController) {
    if (this->rootViewController) {
        // clean up old view controller
        this->rootViewController->viewWillDisappear();
        this->window->removeSubview(this->rootViewController->view);
        this->rootViewController->viewDidDisappear();
    }

    // set up new view controller
    this->rootViewController = viewController;
    this->rootViewController->viewWillAppear();
    this->window->addSubview(this->rootViewController->view);
    this->rootViewController->viewDidAppear();
}
