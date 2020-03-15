#include "qoled.h"
#include "spicsimlink.h"
#include <QPainter>


QOLED::QOLED(QWidget *parent) : QWidget(parent) {
}

QOLED::~QOLED(){

}

void QOLED::paintEvent(QPaintEvent *) {
    QImage screen(128, 64, QImage::Format_Grayscale8);
    screen.fill(0);

    ssd1306_set_flag(SSD1306_FLAG_DIRTY, 0);
    const bool vertical = ssd1306_get_flag(SSD1306_FLAG_SEGMENT_REMAP_0) != 0;
    const bool horizontal = ssd1306_get_flag(SSD1306_FLAG_COM_SCAN_NORMAL) != 0;
    const bool invert = ssd1306_get_flag(SSD1306_FLAG_DISPLAY_INVERTED) == 0;
    const bool active = ssd1306_get_flag(SSD1306_FLAG_DISPLAY_ON);
    if (active){
        uint gray = oled.contrast_register / 2 + 128;
        for (int p = 0; p < ssd1306_pages; p++){
            for (int b = 0; b < 8; b++){
                for (int c = 0; c < ssd1306_cols; c++){
                    if (((oled.vram[p][c] >> b) & 1) == invert){
                        int r = p * 8 + b;
                        int y = horizontal ? (ssd1306_rows - r - 1) : r;
                        int x = vertical ? (ssd1306_cols - c - 1) : c;
                        screen.setPixelColor(x,y,QColor(gray, gray, gray));
                    }
                }
            }
        }
    }

    QPainter painter(this);
    painter.drawImage(QRectF(0, 0, 256, 128), screen);
}
