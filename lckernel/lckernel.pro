#-------------------------------------------------
#
# Project created by QtCreator 2012-03-17T15:29:08
#
#-------------------------------------------------

# Use common project definitions.
include(../settings.pri)

CONFIG      += plugin
TARGET      = $$qtLibraryTarget(lckernel)
TEMPLATE = lib

QT       += core

DEFINES += lckernel_LIBRARY

VERSION=0.0.1

win32 {
    #DESTDIR = ../lcdemo/
}
unix {
    macx {
        DESTDIR = ../demo.app/Contents/MacOS
    } else {
        DESTDIR = ../lcdemo/
    }
}

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
    cad/dochelpers/entitymanagerimpl.cpp \
    cad/document/abstractdocument.cpp \
    cad/dochelpers/undomanagerimpl.cpp \
    cad/primitive/arc.cpp \
    cad/geometry/geocircle.cpp \
    cad/geometry/geoarc.cpp \
    cad/geometry/geoarea.cpp \
    cad/geometry/geovector.cpp \
    cad/geometry/geocoordinate.cpp \
    cad/geometry/geoellipse.cpp \
    cad/primitive/ellipse.cpp \
    cad/primitive/text.cpp \
    cad/math/geointersect.cpp \
    cad/functions/intersect.cpp \
    cad/geometry/geotext.cpp \
    cad/operations/operation.cpp \
    cad/operations/create.cpp \
    cad/operations/delete.cpp \
    cad/operations/move.cpp \
    cad/operations/scale.cpp \
    cad/operations/rotate.cpp \
    cad/operations/trim.cpp \
    cad/operations/copy.cpp \
    cad/functions/ftrim.cpp \
    cad/geometry/geospline.cpp \
    cad/primitive/spline.cpp \
    cad/operations/builder.cpp

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
    cad/events/absoleteentityevent.h \
    cad/primitive/arc.h \
    cad/geometry/geocircle.h \
    cad/geometry/geoarc.h \
    cad/geometry/geotext.h \
    cad/geometry/geoarea.h \
    cad/geometry/geovector.h \
    cad/geometry/geocoordinate.h \
    cad/geometry/geoellipse.h \
    cad/primitive/ellipse.h \
    cad/interface/snapable.h \
    cad/vo/entitycoordinate.h \
    cad/vo/entitydistance.h \
    cad/math/geointersect.h \
    cad/functions/intersect.h \
    cad/interface/entityvisitor.h \
    cad/interface/entitydispatch.h \
    cad/primitive/text.h \
    cad/operations/copy.h \
    cad/operations/create.h \
    cad/operations/delete.h \
    cad/operations/trim.h \
    cad/functions/ftrim.h \
    cad/operations/move.h \
    cad/operations/rotate.h \
    cad/operations/scale.h \
    cad/geometry/geospline.h \
    cad/primitive/spline.h \
    cad/operations/builder.h


