#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T15:29:08
#
#-------------------------------------------------

CONFIG      += plugin
TARGET      = $$qtLibraryTarget(lckernel)
TEMPLATE = lib

QT       += core

win32 {
    #DESTDIR = ../../lcdemo/
}
unix {
    macx {
	DESTDIR = ../../demo.app/Contents/MacOS
    } else {
	DESTDIR = ../../lcdemo/
    }
}

DEFINES += lckernel_LIBRARY

VERSION=0.0.1

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
    cad/base/id.cpp \
    cad/base/metainfo.cpp \
    cad/base/cadentity.cpp \
    cad/primitive/circle.cpp \
    cad/dochelpers/documentimpl.cpp \
    cad/dochelpers/documentlayerimpl.cpp \
    cad/dochelpers/layermanagerimpl.cpp \
    cad/operations/createentities.cpp \
    cad/dochelpers/entitymanagerimpl.cpp \
    cad/document/abstractdocument.cpp \
    cad/dochelpers/undomanagerimpl.cpp \
    cad/operations/deleteentities.cpp \
    cad/primitive/arc.cpp \
    cad/geometry/geocircle.cpp \
    cad/geometry/geoarc.cpp \
    cad/geometry/geoarea.cpp \
    cad/geometry/geovector.cpp \
    cad/geometry/geocoordinate.cpp \
    cad/geometry/geoellipse.cpp \
    cad/primitive/ellipse.cpp \
    cad/operations/trimentity.cpp \
    cad/math/geointersect.cpp \
    cad/functions/intersect.cpp

HEADERS += lckernel.h\
    lckernel_global.h \
    cad/const.h\
    cad/meta/color.h \
    cad/meta/layer.h \
    cad/meta/linewidth.h \
    cad/meta/pen.h \
    cad/primitive/line.h \
    cad/base/id.h \
    cad/base/metainfo.h \
    cad/base/cadentity.h \
    cad/primitive/circle.h \
    cad/document/documentlayer.h \
    cad/document/layermanager.h \
    cad/interface/metatype.h \
    cad/interface/variantable.h \
    cad/dochelpers/documentimpl.h \
    cad/dochelpers/documentlayerimpl.h \
    cad/dochelpers/layermanagerimpl.h \
    cad/document/selectionmanager.h \
    cad/operations/createentities.h \
    cad/operations/operation.h \
    cad/events/beginprocessevent.h \
    cad/events/commitprocessevent.h \
    cad/events/addentityevent.h \
    cad/events/removeentityevent.h \
    cad/events/replaceentityevent.h \
    cad/document/basemanager.h \
    cad/dochelpers/entitymanagerimpl.h \
    cad/document/entitymanager.h \
    cad/document/abstractdocument.h \
    cad/document/undomanager.h \
    cad/operations/undoable.h \
    cad/dochelpers/undomanagerimpl.h \
    cad/operations/deleteentities.h \
    cad/events/absoleteentityevent.h \
    cad/primitive/arc.h \
    cad/geometry/geocircle.h \
    cad/geometry/geoarc.h \
    cad/geometry/geoarea.h \
    cad/geometry/geovector.h \
    cad/geometry/geocoordinate.h \
    cad/geometry/geoellipse.h \
    cad/primitive/ellipse.h \
    cad/interface/snapable.h \
    cad/vo/entitycoordinate.h \
    cad/vo/entitydistance.h \
    cad/operations/trimentity.h \
    cad/math/geointersect.h \
    cad/interface/entityinteraction.h \
    cad/functions/intersect.h
