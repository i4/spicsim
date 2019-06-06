#ifndef QOLED_H
#define QOLED_H

#include <QOpenGLWidget>

class QOLED : public QOpenGLWidget
{
public:
    QOLED(QWidget *parent);
    ~QOLED();

protected:
    void initializeGL();
    void paintGL();
};

#endif // QOLED_H
