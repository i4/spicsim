#include "mainwindow.h"
#include <QApplication>

static void load_icons(){
    Q_INIT_RESOURCE(icons);
}
extern "C" int qt_run(int argc, char *argv[]) {
    load_icons();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
