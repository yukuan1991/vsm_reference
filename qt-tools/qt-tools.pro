QT       += core gui avwidgets
CONFIG += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include (../3rd-party/3rd-party.pri)
include (../qt-tools/qt-tools.pri)
include (../utility/utility.pri)
include (../Shionto.pri)

TARGET = qt-tools
TEMPLATE = app

SOURCES += \
    main.cc
