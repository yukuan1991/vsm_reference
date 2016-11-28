include (../Shionto.pri)
include (../utility/utility.pri)
include (../3rd-party/3rd-party.pri)
include (../qt-tools/qt-tools.pri)

TEMPLATE = app
CONFIG += c++14

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += SERVER_ADDR=\\\"121.41.77.9\\\"


SOURCES += \
    update_app.cc \
    update_widget.cc

HEADERS += \
    update_app.h \
    update_widget.h

FORMS += \
    update_widget.ui

