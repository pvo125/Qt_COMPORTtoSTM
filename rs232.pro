#-------------------------------------------------
#
# Project created by QtCreator 2016-11-05T09:08:57
#
#-------------------------------------------------

QT       += core gui
QT      +=serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = rs232
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RC_FILE +=rsr232.rc
