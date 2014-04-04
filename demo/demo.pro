#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T21:50:29
#
#-------------------------------------------------

# Use common project definitions.
include(../settings.pri)

QT       += core gui opengl
TEMPLATE = app

win32 {
    DESTDIR = ../lcdemo
}

unix {
    macx {
	TARGET = ../demo
	CONFIG(debug, debug|release) {
	    LIBS += -L$$PWD/../lckernel -llckernel_debug
            LIBS += -llcadviewerplugin_debug
            LIBS += -llcadluascript_debug
        } else {
	    LIBS += -L$$PWD/../lckernel -llckernel
            LIBS += -L$$PWD/../lcadviewer -llcadviewerplugin
            LIBS += -L$$PWD/../lcadluascript -llcadluascriptplugin
        }
    } else {
	TARGET = ../lcdemo/demo
	LIBS += -L$$PWD/../lckernel -llckernel
        LIBS += -L$$PWD/../lcadviewer -llcadviewerplugin
        LIBS += -L$$PWD/../lcadluascript -llcadluascript
    }
}

win32 {
    QMAKE_LIBDIR += ../lcdemo
}
unix {
    macx {
        QMAKE_LIBDIR += ../demo.app/Contents/MacOS
    } else {
        QMAKE_LIBDIR += ../lcdemo
    }
}


INCLUDEPATH += $$PWD/../lckernel
DEPENDPATH += $$PWD/../lckernel

INCLUDEPATH += $$PWD/../lcadviewer
DEPENDPATH += $$PWD/../lcadviewer

INCLUDEPATH += $$PWD/../lcadluascript
DEPENDPATH += $$PWD/../lcadluascript


SOURCES += main.cpp\
        mainwindow.cpp \
    cadmdichild.cpp \
    ui/lcmaintoolbar.cpp \
    operations/linecreateoperation.cpp \
    ui/clicommand.cpp \
    operations/qsnappedstate.cpp \
    operations/operationmanager.cpp \
    operations/circlecreateoperation.cpp \
    operations/trimoperation.cpp \
    ui/luascript.cpp

HEADERS  += mainwindow.h \
    cadmdichild.h \
    ui/lcmaintoolbar.h \
    const.h \
    operations/linecreateoperation.h \
    imainwindow.h \
    operations/groupoperations.h \
    ui/clicommand.h \
    operations/qsnappedstate.h \
    operations/operationmanager.h \
    operations/circlecreateoperation.h \
    operations/trimoperation.h \
    operations/guioperation.h \
    operations/guioperationfinishedevent.h \
    ui/luascript.h

FORMS    += mainwindow.ui \
    cadmdichild.ui \
    ui/lcmaintoolbar.ui \
    ui/clicommand.ui \
    ui/form.ui \
    ui/luascript.ui

RESOURCES += \
    ui/resource.qrc




