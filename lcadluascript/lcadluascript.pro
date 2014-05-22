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
        CONFIG+=link_pkgconfig
        packagesExist(lua5.2) {
		PKGCONFIG += lua5.2
	} else:packagesExist(lua-5.2) {
		PKGCONFIG += lua-5.2
	} else {
		PKGCONFIG += lua
	}
        PKGCONFIG += liblog4cxx
    } else {
        DESTDIR = ../lcdemo
        LIBS += -L$$DESTDIR -llckernel
        CONFIG+=link_pkgconfig
        packagesExist(lua5.2) {
		PKGCONFIG += lua5.2
	} else:packagesExist(lua-5.2) {
		PKGCONFIG += lua-5.2
	} else {
		PKGCONFIG += lua
	}
        PKGCONFIG += liblog4cxx
    }
}

INCLUDEPATH += $$PWD/../lckernel
DEPENDPATH += $$PWD/../lckernel

SOURCES += \
        lcadluascript.cpp \
    cad/lualibrecadbridge.cpp

HEADERS += \
        const.h \
        LuaIntf.h \
        lcadluascript.h\
        lcadluascript_global.h \
    cad/lualibrecadbridge.h \
    cad/timer.h
