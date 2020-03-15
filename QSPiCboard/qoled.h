#ifndef QOLED_H
#define QOLED_H

#include <QWidget>

class QOLED : public QWidget
{
public:
    QOLED(QWidget *parent);
    ~QOLED();

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // QOLED_H
