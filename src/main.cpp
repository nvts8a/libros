#include "OpenBookApplication.h"

std::shared_ptr<OpenBookApplication> application(nullptr);

void setup() {
    application = std::make_shared<OpenBookApplication>();
}

void loop() {
    application->run();
}