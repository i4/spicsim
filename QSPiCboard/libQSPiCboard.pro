QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QSPiCboard
TEMPLATE = lib

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += staticlib

SOURCES += \
        fix_vcd.cpp \
        mainwindow.cpp \
        qled.cpp \
        qimg.cpp \
        qsevenseg.cpp \
        qoled.cpp \
        spicsimlink.cpp

HEADERS += \
        fix_vcd.h \
        mainwindow.h \
        qled.h \
        qimg.h \
        qsevenseg.h \
        qoled.h \
        spicsimlink.h

INCLUDEPATH += ..

FORMS += mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc \
    skins.qrc
