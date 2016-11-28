#-------------------------------------------------
#
# Project created by QtCreator 2016-08-24T21:41:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = login_dialog
TEMPLATE = app

include (../Shionto.pri)
include (../login/login.pri)
include (../3rd-party/3rd-party.pri)
include (../utility/utility.pri)

DEFINES += IMAGE_PATH=\\\"./images\\\"

SOURCES += main.cpp

