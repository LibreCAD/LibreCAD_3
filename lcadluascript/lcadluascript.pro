#-------------------------------------------------
#
# Project created by QtCreator 2014-03-30T18:01:48
#
#-------------------------------------------------
include(../settings.pri)

QT       -= gui

TARGET = $$qtLibraryTarget(lcadluascript)
TEMPLATE = lib

DEFINES += LCADLUASCRIPT_LIBRARY

include(lua.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}

unix {
    macx {
        QT_CONFIG -= no-pkg-config
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

SOURCES += \
        lcadluascript.cpp \
    cad/lualibrecadbridge.cpp

HEADERS += \
        const.h \
        LuaBridge/LuaBridge.h \
        LuaBridge/RefCountedObject.h \
        LuaBridge/RefCountedPtr.h \
        lcadluascript.h\
        lcadluascript_global.h \
        cad/primitive/lualine.h \
    cad/lualibrecadbridge.h
