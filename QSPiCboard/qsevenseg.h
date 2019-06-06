#ifndef QSEVENSEG_H
#define QSEVENSEG_H

#include <QWidget>

class QSevenSeg : public QWidget
{
    Q_OBJECT
public:
    explicit QSevenSeg(QWidget *parent = nullptr);
    enum Segment {
        SEGMENT_0 = 0,
        SEGMENT_1,
        SEGMENT_2,
        SEGMENT_3,
        SEGMENT_4,
        SEGMENT_5,
        SEGMENT_6,
        SEGMENT_7,
        SEGMENTS
    };

    void setSegment(enum Segment, qreal value);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor segments[SEGMENTS];
};

#endif // QSEVENSEG_H
