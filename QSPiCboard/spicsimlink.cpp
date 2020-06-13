#include "mainwindow.h"
#include <QApplication>

static void load_icons(){
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(skins);
}
extern "C" int qt_run(int argc, char *argv[]) {
    load_icons();
    QApplication::setAttribute(Qt::AA_Use96Dpi);
    QApplication a(argc, argv);
    QFont font = qApp->font();
    font.setPixelSize(12);
    qApp->setFont(font);
    MainWindow w;
    w.show();
    return a.exec();
}
