#include "qled.h"

#include <QPainter>

static QColor borderColor(100, 100, 100);

QLED::QLED(QWidget *parent) : QWidget(parent) {
}

void QLED::setColor(enum QLED::Color color){
    switch (color) {
        case RED: fillColor.setRgb(0xe6, 0x4a, 0x19); break;
        case GREEN: fillColor.setRgb(0x38, 0x8e, 0x3c); break;
        case BLUE: fillColor.setRgb(0x19, 0x76, 0xd2); break;
        case YELLOW: fillColor.setRgb(0xfb, 0xc0, 0x2d); break;
    }
}

void QLED::setLightness(qreal value){
    fillColor.setAlphaF(value);
    update();
}

void QLED::paintEvent(QPaintEvent *) {

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(borderColor);
    painter.setBrush(fillColor);

    painter.drawEllipse(10, 5, 20,20);
}
