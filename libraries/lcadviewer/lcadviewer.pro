CONFIG      += designer plugin
CONFIG +=
TARGET      = $$qtLibraryTarget(lcadviewerplugin)
TEMPLATE    = lib

QT += opengl

VERSION=0.0.1

GENERATED_DIR = ../../generated/lib/lcadviewer
# Use common project definitions.
include(../../settings.pro)
include(../../common.pro)

DESTDIR = ../../demo.app/Contents/MacOS

HEADERS     = lcadviewerplugin.h
SOURCES     = lcadviewerplugin.cpp

RESOURCES   = icons.qrc
LIBS        += -L. 

INCLUDEPATH += "../lckernel"
LIBS+=  -L../../demo.app/Contents/MacOS -llckernel

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(lcadviewer.pri)
