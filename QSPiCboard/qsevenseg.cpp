#include "qsevenseg.h"

#include <QPainter>

QSevenSeg::QSevenSeg(QWidget *parent) : QWidget(parent), border(100, 100, 100), fill(0xd3, 0x2f, 0x2f) {
    for (unsigned s = 0; s < SEGMENTS; s++)
        segments[s].setRgb(0xd3, 0x2f, 0x2f);
}


void QSevenSeg::setSegment(enum QSevenSeg::Segment segment, qreal value){
    if (segment < SEGMENTS){
        if (segments[segment].alphaF() == value) {
            return;
        }
        segments[segment].setAlphaF(value);
        update();
    }
}

void QSevenSeg::setSize(int size, int offset, bool showBorder){
    this->size = size;
    this->offset = offset;
    this->showBorder = showBorder;
    update();
}

void QSevenSeg::setColor(QColor fill, QColor border){
    this->fill = fill;
    this->border = border;
    update();
}

void QSevenSeg::paintEvent(QPaintEvent *) {
    const QPoint segPoly[6] = {
        QPoint(0, -4 * size),
        QPoint(size, -3 * size),
        QPoint(size, 3 * size),
        QPoint(0, 4 * size),
        QPoint(-1 * size, 3 * size),
        QPoint(-1 * size, -3 * size),
    };

    int pos = 4 * size + offset;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);

    if (showBorder)
        painter.setPen(border);
    else
        painter.setPen(Qt::NoPen);

    painter.save();
    painter.translate(-1 * pos, -1 * pos);
    painter.setBrush(segments[SEGMENT_5]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(pos, -1 * pos);
    painter.setBrush(segments[SEGMENT_1]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(-1 * pos, pos);
    painter.setBrush(segments[SEGMENT_4]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(pos, pos);
    painter.setBrush(segments[SEGMENT_2]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.rotate(90.0);

    painter.save();
    painter.setBrush(segments[SEGMENT_6]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(2 * pos, 0);
    painter.setBrush(segments[SEGMENT_3]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(-2 * pos, 0);
    painter.setBrush(segments[SEGMENT_0]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();
}
