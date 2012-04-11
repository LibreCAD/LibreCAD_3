#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T21:50:29
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = ../demo
TEMPLATE = app

CONFIG += static

# Store intermedia stuff somewhere else
GENERATED_DIR = ../generated/demo

# Use common project definitions.
include(../settings.pro)
include(../common.pro)

INCLUDEPATH += "../libraries/lckernel"
INCLUDEPATH += "../libraries/lcadviewer"

LIBS+=  -L../demo.app/Contents/MacOS -llckernel  -llcadviewerplugin

SOURCES += main.cpp\
        mainwindow.cpp \
    cadmdichild.cpp

HEADERS  += mainwindow.h \
    cadmdichild.h

FORMS    += mainwindow.ui \
    cadmdichild.ui
