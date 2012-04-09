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
    drawitems/gradientbackground.h \
    graphicsitems/lcviewer.h \
    graphicsitems/lcgraphicsitem.h \
    graphicsitems/lclineitem.h \
    graphicsitems/lccircleitem.h \
    graphicsitems/lcarcitem.h \
    graphicsitems/lcellipseitem.h \
    const.h \
    drawitems/metricgrid.h \
    drawitems/cursor.h \
    drawitems/lcviewercursoritem.h \
    drawitems/lcviewerdrawitem.h \
    qcachedgraphicsview.h \
    events/MouseMoveEvent.h \
    helpers/snapmanager.h \
    helpers/snapmanagerimpl.h \
    events/snappointevent.h

SOURCES     = lcadviewerplugin.cpp \
    lcadviewer.cpp \
    scenemanager.cpp \
    drawitems/gradientbackground.cpp \
    graphicsitems/lcgraphicsitem.cpp \
    graphicsitems/lclineitem.cpp \
    graphicsitems/lccircleitem.cpp \
    graphicsitems/lcarcitem.cpp \
    graphicsitems/lcellipseitem.cpp \
    drawitems/metricgrid.cpp \
    drawitems/cursor.cpp \
    qcachedgraphicsview.cpp \
    helpers/snapmanagerimpl.cpp


RESOURCES   = icons.qrc
LIBS        += -L. 

INCLUDEPATH += "../lckernel"
LIBS+=  -L../../demo.app/Contents/MacOS -llckernel

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target
