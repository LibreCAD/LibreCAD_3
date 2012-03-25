#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T21:50:29
#
#-------------------------------------------------

QT       += core gui

TARGET = ../demo
TEMPLATE = app

# Store intermedia stuff somewhere else
GENERATED_DIR = ../generated/demo
# Use common project definitions.
include(../settings.pro)
include(../common.pro)

INCLUDEPATH += "../libraries/lckernel"
INCLUDEPATH += "../libraries/lcadviewer"

LIBS+=  -L../generated/lib -llckernel  -llcadviewerplugin

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
