#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T15:29:08
#
#------------------------------------------------

# Use common project definitions.
include(../settings.pri)

TARGET      = $$qtLibraryTarget(lcadviewerplugin)
TEMPLATE    = lib

VERSION=0.0.1

#ifdef QT_VERSION > 0x040806
    QT += opengl designer
    CONFIG += plugin
#else
    QT += opengl
    CONFIG += designer plugin
#endif

OTHER_FILES = CustomWidget.json

VERSION=0.0.1

RESOURCES   = icons.qrc

win32 {
    #DESTDIR = ../lcdemo/
}
unix {
    macx {
        DESTDIR = ../demo.app/Contents/MacOS
	CONFIG(debug, debug|release) {
	    LIBS += -L$$DESTDIR -llckernel_debug
	} else {
	    LIBS += -L$$DESTDIR -llckernel
	}
    } else {
        DESTDIR = ../lcdemo
	LIBS += -L$$DESTDIR -llckernel
    }
}



INCLUDEPATH += $$PWD/../lckernel
DEPENDPATH += $$PWD/../lckernel



HEADERS     = lcadviewerplugin.h \
    lcadviewer.h \
    scenemanager.h \
    drawitems/gradientbackground.h \
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
    helpers/snapmanager.h \
    helpers/snapmanagerimpl.h \
    events/snappointevent.h \
    events/drawevent.h \
    events/mousemoveevent.h \
    helpers/selectionmanagerimpl.h \
    events/mousereleaseevent.h

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
    helpers/snapmanagerimpl.cpp \
    helpers/selectionmanagerimpl.cpp


