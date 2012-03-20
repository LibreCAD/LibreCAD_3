#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T15:29:08
#
#-------------------------------------------------

QT       -=

TARGET = lckernel
TEMPLATE = lib

DESTDIR = ../../generated/lib

#CONFIG += static warn_on
DEFINES += lckernel_LIBRARY

VERSION=0.0.1

DLL_NAME = lckernel
TARGET = $$DLL_NAME

GENERATED_DIR = ../../generated/lib/lckernel
# Use common project definitions.
include(../../settings.pro)
include(../../common.pro)


SOURCES += lckernel.cpp \
   cad/meta/color.cpp \
   cad/meta/layer.cpp \
   cad/meta/linewidth.cpp \
   cad/meta/pen.cpp \
   cad/primitive/line.cpp \
    cad/geometry/point.cpp \
    cad/base/id.cpp \
    cad/document.cpp \
    cad/base/metainfo.cpp \
    cad/interface/variantable.cpp \
    cad/interface/metatype.cpp \
    cad/base/cadentity.cpp \
    cad/primitive/circle.cpp

HEADERS += lckernel.h\
    lckernel_global.h \
    cad/meta/color.h \
    cad/meta/layer.h \
    cad/meta/linewidth.h \
    cad/meta/pen.h \
    cad/primitive/line.h \
    cad/geometry/point.h \
    cad/base/id.h \
    cad/document.h \
    cad/base/metainfo.h \
    cad/interface/variantable.h \
    cad/interface/metatype.h \
    cad/base/cadentity.h \
    cad/primitive/circle.h


unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

OTHER_FILES += \
    cad/const.h
