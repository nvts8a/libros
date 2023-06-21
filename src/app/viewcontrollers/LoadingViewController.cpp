#include "LoadingViewController.h"

#include "Adafruit_EPD.h"
#include "OpenBookApplication.h"

LoadingViewController::LoadingViewController(std::shared_ptr<Application> application, std::string defaultMessage) : ViewController(application) {
    this->defaultMessage = defaultMessage;
}

void LoadingViewController::createView() {
    ViewController::createView();
    view = std::make_shared<View>(MakeRect(0, 0, 300, 400));
    view->addSubview(SPLASH);
    view->addSubview(VERSION);
    messageLabel = std::make_shared<Label>(MESSAGE_RECT, this->defaultMessage);
    view->addSubview(messageLabel);
}

void LoadingViewController::setMessage(std::string message) {
    // Pad the message
    message.insert(0, 3, ' '); message.append(3, '.');
    // Center the message in the view
    int16_t indent = ((MESSAGE_RECT.size.width/6) - message.length()) / 2;
    message.insert(0, indent, ' ');

    messageLabel->setText(message);
}
