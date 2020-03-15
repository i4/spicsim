#include "qimg.h"


#include <QPainter>

QIMG::QIMG(QWidget *parent) : QWidget(parent), opacity(0.0) {
}

void QIMG::setImage(const QString& ressource){
    pixmap.load(ressource);
    update();
}

void QIMG::setLightness(qreal value){
    if (opacity == value) {
        return;
    }
    opacity = value;
    update();
}

void QIMG::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setOpacity(opacity);
    painter.drawPixmap(0, 0, pixmap);
}
