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

HEADERS     = lcadviewerplugin.h \
    lcadviewer.h \
    scenemanager.h \
    drawitems/lcadviewerdrawitem.h \
    drawitems/gradientbackground.h \
    graphicsitems/lcviewer.h \
    graphicsitems/lcgraphicsitem.h \
    graphicsitems/lclineitem.h \
    graphicsitems/lccircleitem.h \
    graphicsitems/lcarcitem.h \
    graphicsitems/lcellipseitem.h \
    const.h \
    drawitems/metricgrid.h

SOURCES     = lcadviewerplugin.cpp \
    lcadviewer.cpp \
    scenemanager.cpp \
    drawitems/gradientbackground.cpp \
    graphicsitems/lcgraphicsitem.cpp \
    graphicsitems/lclineitem.cpp \
    graphicsitems/lccircleitem.cpp \
    graphicsitems/lcarcitem.cpp \
    graphicsitems/lcellipseitem.cpp \
    drawitems/metricgrid.cpp


RESOURCES   = icons.qrc
LIBS        += -L. 

INCLUDEPATH += "../lckernel"
LIBS+=  -L../../demo.app/Contents/MacOS -llckernel

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target
