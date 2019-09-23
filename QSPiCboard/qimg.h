#ifndef QIMG_H
#define QIMG_H

#include <QWidget>
#include <QPixmap>

class QIMG : public QWidget
{
    Q_OBJECT
public:
    explicit QIMG(QWidget *parent = nullptr);
    void setImage(const QString& ressource);
    void setLightness(qreal value);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    qreal opacity;
    QPixmap pixmap;
};

#endif // QIMG_H
