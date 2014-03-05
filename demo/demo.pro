#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T21:50:29
#
#-------------------------------------------------

# Use common project definitions.
include(../settings.pro)
include(../common.pro)

QT       += core gui opengl

TEMPLATE = app

# Store intermedia stuff somewhere else
GENERATED_DIR = ../generated/demo


INCLUDEPATH += "../libraries/lckernel"
INCLUDEPATH += "../libraries/lcadviewer"

CONFIG(debug, debug|release) {
    LIBS+= -llckernel_debug -llcadviewerplugin_debug
} else {
    LIBS+= -llckernel -llcadviewerplugin
}

win32 {
    DESTDIR = ../../lcdemo/
}
unix {
    macx {
	QMAKE_LIBDIR += ../demo.app/Contents/MacOS
	TARGET = ../demo
    } else {
	QMAKE_LIBDIR += ../lcdemo
	TARGET = ../lcdemo/demo
    }
}


SOURCES += main.cpp\
        mainwindow.cpp \
    cadmdichild.cpp \
    ui/lcmaintoolbar.cpp \
    operations/linecreateoperation.cpp \
    ui/clicommand.cpp \
    operations/qsnappedstate.cpp \
    operations/operationmanager.cpp \
    operations/circlecreateoperation.cpp \
    operations/trimoperation.cpp

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
    operations/guioperationfinishedevent.h

FORMS    += mainwindow.ui \
    cadmdichild.ui \
    ui/lcmaintoolbar.ui \
    ui/clicommand.ui \
    ui/form.ui

RESOURCES += \
    ui/resource.qrc
