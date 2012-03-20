#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T21:50:29
#
#-------------------------------------------------

QT       += core gui

TARGET = demo
TEMPLATE = app


INCLUDEPATH += "../libraries/lckernel"

LIBS+=  -L../generated/lib -llckernel

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
