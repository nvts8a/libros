#ifndef Widgets_h
#define Widgets_h

#include "Application.h"

typedef enum {
    CellSelectionStyleInvert,
    CellSelectionStyleIndicatorLeading,
    CellSelectionStyleIndicatorTrailing,
    CellSelectionStyleIndicatorAbove,
    CellSelectionStyleIndicatorBelow
} CellSelectionStyle;

class BitmapView : public View {
public:
    BitmapView(Rect rect, const unsigned char *bitmap);
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
protected:
    const unsigned char *bitmap;
};

class Button : public Control {
public:
    Button(Rect rect, std::string text);
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
protected:
    std::string text;
};

class TypesetterButton : public Button {
public:
    TypesetterButton(Rect rect, std::string text);
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
};

class HatchedView : public View {
public:
    HatchedView(Rect rect, uint16_t color);
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
};

class BorderedView : public View {
public:
    BorderedView(Rect rect);
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
};

class ProgressView : public View {
public:
    ProgressView(Rect rect) : View(rect) {};
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
    void setProgress(float value);
    float getProgress();
protected:
    float progress = 0;
};

class GrayscaleImageView : public View {
public:
    GrayscaleImageView(Rect rect, const unsigned char *image);
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
protected:
    const unsigned char *image;
};

class Label : public View {
public:
    Label(Rect rect, std::string text);
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
    void setText(std::string text);
protected:
    std::string text;
};

class TypesetterLabel : public Label {
public:
    TypesetterLabel(Rect rect, std::string text);
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
    void setWordWrap(bool value);
    void setBold(bool value);
    void setItalic(bool value);
    void setTextSize(uint16_t value);
    void setLineSpacing(uint16_t value);
    void setParagraphSpacing(uint16_t value);
protected:
    bool wrap = false;
    bool bold = false;
    bool italic = false;
    uint16_t textSize = 1;
    uint16_t lineSpacing = 0;
    uint16_t paragraphSpacing = 0;
};


class TableCell : public Control {
public:
    TableCell(Rect rect, std::string text, CellSelectionStyle selectionStyle);
    void draw(Adafruit_GFX *display, int16_t x, int16_t y) override;
    void didBecomeFocused() override;
    void didResignFocus() override;
protected:
    Rect _indicatorRect();
    std::string text;
    CellSelectionStyle selectionStyle;
};

class Table : public Control {
public:
    Table(Rect rect, int16_t cellHeight, CellSelectionStyle selectionStyle);
    void setItems(std::vector<std::string> items);
    bool handleEvent(Event event) override;
    bool becomeFocused() override;
    int32_t getSelectedIndex();
protected:
    void updateCells();
    int32_t selectedIndex = 0;
    int16_t cellHeight;
    int16_t cellsPerPage;
    int16_t startOffset = 0;
    std::vector<std::string> items;
    std::string text;
    CellSelectionStyle selectionStyle;
};

#endif // Widgets_h