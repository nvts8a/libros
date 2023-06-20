#include "Widgets.h"
#include "Adafruit_EPD.h"

GrayscaleImageView::GrayscaleImageView(Rect rect, const unsigned char *image) : View(rect) {
    this->image = image;
}

void GrayscaleImageView::draw(Adafruit_GFX *display, int16_t x, int16_t y) {
    int16_t byteWidth = (this->frame.size.width * 2 + 7) / 8;
    uint8_t byte = 0;
    uint16_t color;

    for(int16_t j = 0; j < this->frame.size.height; j++) {
        for(int16_t i=0; i < this->frame.size.width * 2; i += 2 ) {
            if (i & 7){
                byte <<= 2;
            }
            else {
                byte = this->image[j * byteWidth + i / 8];
            }
            switch((byte & 0xC0) >> 6) {
                case 0:
                    color = EPD_BLACK;
                    break;
                case 1:
                    color = EPD_DARK;
                    break;
                case 2:
                    color = EPD_LIGHT;
                    break;
                case 3:
                    color = EPD_WHITE;
                    break;
                default:
                    break;
            }
            display->writePixel(x + this->frame.origin.x + i / 2, y + this->frame.origin.y + j, color);
        }
    }
}
