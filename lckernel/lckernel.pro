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

DEFINES += lckernel_LIBRARY

VERSION=0.0.1

win32 {
    #DESTDIR = ../lcdemo/
}
unix {
    macx {
        DESTDIR = ../demo.app/Contents/MacOS

        CONFIG+=link_pkgconfig
        PKGCONFIG += liblog4cxx
    } else {
        DESTDIR = ../lcdemo/

        CONFIG+=link_pkgconfig
        PKGCONFIG += liblog4cxx
    }
}

SOURCES += lckernel.cpp \
   cad/meta/layer.cpp \
   cad/meta/linewidth.cpp \
   cad/meta/pen.cpp \
   cad/primitive/line.cpp \
    cad/base/id.cpp \
    cad/base/metainfo.cpp \
    cad/base/cadentity.cpp \
    cad/primitive/circle.cpp \
    cad/dochelpers/documentimpl.cpp \
    cad/dochelpers/undomanagerimpl.cpp \
    cad/primitive/arc.cpp \
    cad/geometry/geocircle.cpp \
    cad/geometry/geoarc.cpp \
    cad/geometry/geoellipse.cpp \
    cad/primitive/ellipse.cpp \
    cad/primitive/text.cpp \
    cad/functions/intersect.cpp \
    cad/geometry/geotext.cpp \
    cad/operations/trim.cpp \
    cad/functions/ftrim.cpp \
    cad/geometry/geospline.cpp \
    cad/primitive/spline.cpp \
    cad/operations/builder.cpp \
    cad/operations/documentoperation.cpp \
    cad/operations/builderops.cpp \
    cad/document/document.cpp \
    cad/dochelpers/storagemanagerimpl.cpp \
    cad/dochelpers/entitycontainer.cpp \
    cad/dochelpers/quadtree.cpp \
    cad/math/lcmath.cpp \
    cad/geometry/geomtext.cpp \
    cad/primitive/mtext.cpp \
    cad/geometry/geodimension.cpp \
    cad/primitive/dimension.cpp \
    cad/primitive/dimaligned.cpp \
    cad/geometry/geodimaligned.cpp \
    cad/geometry/geodimangular.cpp \
    cad/primitive/dimangular.cpp


HEADERS += \
    nano-signal-slot/nano_signal_slot.hpp \
    lckernel.h\
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
    cad/interface/metatype.h \
    cad/interface/variantable.h \
    cad/dochelpers/documentimpl.h \
    cad/document/selectionmanager.h \
    cad/events/beginprocessevent.h \
    cad/events/commitprocessevent.h \
    cad/events/addentityevent.h \
    cad/events/removeentityevent.h \
    cad/events/replaceentityevent.h \
    cad/document/undomanager.h \
    cad/operations/undoable.h \
    cad/dochelpers/undomanagerimpl.h \
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
    cad/functions/intersect.h \
    cad/interface/entityvisitor.h \
    cad/interface/entitydispatch.h \
    cad/primitive/text.h \
    cad/operations/trim.h \
    cad/functions/ftrim.h \
    cad/geometry/geospline.h \
    cad/primitive/spline.h \
    cad/operations/builder.h \
    cad/operations/documentoperation.h \
    cad/operations/builderops.h \
    cad/dochelpers/entitycontainer.h \
    cad/document/document.h \
    cad/events/removelayerevent.h \
    cad/events/addlayerevent.h \
    cad/events/replacelayerevent.h \
    cad/dochelpers/storagemanagerimpl.h \
    cad/document/storagemanager.h \
    cad/dochelpers/quadtree.h \
    cad/math/lcmath.h \
    cad/geometry/geomtext.h \
    cad/primitive/mtext.h \
    cad/geometry/geodimension.h \
    cad/primitive/dimension.h \
    cad/primitive/dimaligned.h \
    cad/geometry/geodimaligned.h \
    cad/primitive/dimangular.h \
    cad/geometry/geodimangular.h
