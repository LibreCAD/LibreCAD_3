#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T15:29:08
#
#------------------------------------------------

# Use common project definitions.
include(../settings.pri)

TARGET      = $$qtLibraryTarget(lcadcairoviewerplugin)
TEMPLATE    = lib

VERSION=0.0.1


lessThan(QT_MAJOR_VERSION, 5) {
    QT += opengl
    CONFIG += designer plugin
} else {
    QT += opengl designer
    CONFIG += plugin
}

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



HEADERS     = lcadcairoviewerplugin.h \
    lcadcairoviewer.h \
    scenemanager.h \
    drawitems/gradientbackground.h \
    const.h \
    drawitems/metricgrid.h \
    drawitems/cursor.h \
    drawitems/lcviewercursoritem.h \
    drawitems/lcviewerdrawitem.h \
    helpers/snapmanager.h \
    helpers/snapmanagerimpl.h \
    events/snappointevent.h \
    events/drawevent.h \
    events/mousemoveevent.h \
    helpers/selectionmanagerimpl.h \
    events/mousereleaseevent.h \
    events/selecteditemsevent.h

SOURCES     = lcadcairoviewerplugin.cpp \
    lcadcairoviewer.cpp \
    scenemanager.cpp \
    drawitems/gradientbackground.cpp \
    drawitems/metricgrid.cpp \
    drawitems/cursor.cpp \
    helpers/snapmanagerimpl.cpp \
    helpers/selectionmanagerimpl.cpp


