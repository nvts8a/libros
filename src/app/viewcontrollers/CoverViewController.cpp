#include "CoverViewController.h"
#include "Widgets.h"
#include "OpenBookDevice.h"

CoverViewController::CoverViewController(std::shared_ptr<Application> application) : ViewController(application) {}

void CoverViewController::createView() {
    ViewController::createView();
    Rect viewRect = MakeRect(0, 0, 300, 400);
    char *bufferTemp; long imageSize;
    FILE *image = fopen("/2br02b.bmp", "rb");

    fseek(image, 0, SEEK_END); imageSize = ftell(image);
    fread(bufferTemp, imageSize, 1, image); fclose(image);

    const unsigned char* buffer = reinterpret_cast<const unsigned char *>( bufferTemp );

    view = std::make_shared<GrayscaleImageView>(viewRect, buffer);
}