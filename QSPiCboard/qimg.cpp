#include "qimg.h"


#include <QPainter>

QIMG::QIMG(QWidget *parent) : QWidget(parent), opacity(0.0) {
}

void QIMG::setImage(const QString& ressource){
    pixmap.load(ressource);
}

void QIMG::setLightness(qreal value){
    opacity = value;
}

void QIMG::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setOpacity(opacity);
    painter.drawPixmap(0, 0, pixmap);
}
