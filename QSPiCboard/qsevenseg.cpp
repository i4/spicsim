#include "qsevenseg.h"

#include <QPainter>

QSevenSeg::QSevenSeg(QWidget *parent) : QWidget(parent){
    for (unsigned s = 0; s < SEGMENTS; s++)
        segments[s].setRgb(0xd3, 0x2f, 0x2f);
}


void QSevenSeg::setSegment(enum QSevenSeg::Segment segment, qreal value){
    if (segment < SEGMENTS){
        segments[segment].setAlphaF(value);
        update();
    }
}


void QSevenSeg::paintEvent(QPaintEvent *) {
    static QColor borderColor(100, 100, 100);
    QColor fillColor(0xd3, 0x2f, 0x2f);

    static const QPoint segPoly[6] = {
        QPoint(0, -20),
        QPoint(5, -15),
        QPoint(5, 15),
        QPoint(0, 20),
        QPoint(-5, 15),
        QPoint(-5, -15),
    };

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width() / 2, height() / 2);

    painter.setPen(borderColor);

    painter.save();
    painter.translate(-22, -22);
    painter.setBrush(segments[SEGMENT_5]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(22, -22);
    painter.setBrush(segments[SEGMENT_1]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(-22, 22);
    painter.setBrush(segments[SEGMENT_4]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(22, 22);
    painter.setBrush(segments[SEGMENT_2]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.rotate(90.0);

    painter.save();
    painter.setBrush(segments[SEGMENT_6]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(44, 0);
    painter.setBrush(segments[SEGMENT_3]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();

    painter.save();
    painter.translate(-44, 0);
    painter.setBrush(segments[SEGMENT_0]);
    painter.drawConvexPolygon(segPoly, 6);
    painter.restore();
}
