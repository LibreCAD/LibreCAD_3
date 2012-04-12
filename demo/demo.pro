#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T21:50:29
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = ../lcdemo/demo
TEMPLATE = app

# Store intermedia stuff somewhere else
GENERATED_DIR = ../generated/demo

# Use common project definitions.
include(../settings.pro)
include(../common.pro)

INCLUDEPATH += "../libraries/lckernel"
INCLUDEPATH += "../libraries/lcadviewer"

win32 {
    DESTDIR = ../../lcdemo/
}
unix {
    macx {
        LIBS+=  -L../demo.app/Contents/MacOS -llckernel  -llcadviewerplugin
    } else {
        LIBS+=  -L../lcdemo -llckernel  -llcadviewerplugin
    }
}


SOURCES += main.cpp\
        mainwindow.cpp \
    cadmdichild.cpp

HEADERS  += mainwindow.h \
    cadmdichild.h

FORMS    += mainwindow.ui \
    cadmdichild.ui
