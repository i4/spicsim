#ifndef QLED_H
#define QLED_H

#include <QWidget>

class QLED : public QWidget
{
    Q_OBJECT
public:
    explicit QLED(QWidget *parent = nullptr);
    enum Color {
        RED = 0, YELLOW, GREEN, BLUE
    };

    void setColor(enum Color color);
    void setSize(int size);
    void setLightness(qreal value);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QColor fillColor;
    int size = 20;
};

#endif // QLED_H
