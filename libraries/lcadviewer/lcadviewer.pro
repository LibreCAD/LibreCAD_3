CONFIG      += designer plugin
CONFIG += static
TARGET      = $$qtLibraryTarget(lcadviewerplugin)
TEMPLATE    = lib

GENERATED_DIR = ../../generated/lib/lcadviewer
# Use common project definitions.
include(../../settings.pro)
include(../../common.pro)

DESTDIR = ../../generated/lib

HEADERS     = lcadviewerplugin.h
SOURCES     = lcadviewerplugin.cpp
RESOURCES   = icons.qrc
LIBS        += -L. 

INCLUDEPATH += "../lckernel"
LIBS+=  -L../generated/lib -llckernel

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(lcadviewer.pri)
