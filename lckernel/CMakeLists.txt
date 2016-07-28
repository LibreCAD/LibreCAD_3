CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCKERNEL)
ADD_DEFINITIONS(-std=c++14)
ADD_DEFINITIONS("-Wall")
#ADD_DEFINITIONS("-Wno-missing-braces")

message(${CMAKE_SOURCE_DIR})

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# SOURCE FILES
set(lckernel_srcs
cad/base/cadentity.cpp
cad/base/id.cpp
cad/base/metainfo.cpp
cad/base/visitor.h
cad/dochelpers/documentimpl.cpp
cad/dochelpers/entitycontainer.cpp
cad/dochelpers/quadtree.cpp
cad/dochelpers/storagemanagerimpl.cpp
cad/dochelpers/undomanagerimpl.cpp
cad/document/document.cpp
cad/functions/intersect.cpp
cad/geometry/geoarc.cpp
cad/geometry/geocircle.cpp
cad/geometry/geocoordinate.cpp
cad/geometry/geoellipse.cpp
cad/geometry/geospline.cpp
cad/geometry/geobezier.cpp
cad/geometry/geobeziercubic.cpp
cad/math/lcmath.cpp
cad/math/equation.cpp
cad/math/intersectionhandler.cpp
cad/meta/layer.cpp
cad/meta/icolor.cpp
cad/meta/metalinewidth.cpp
cad/meta/dxflinepattern.cpp
cad/operations/builder.cpp
cad/operations/builderops.cpp
cad/operations/documentoperation.cpp
cad/operations/layerops.cpp
cad/operations/linepatternops.cpp
cad/primitive/arc.cpp
cad/primitive/circle.cpp
cad/primitive/point.cpp
cad/primitive/ellipse.cpp
cad/primitive/line.cpp
cad/primitive/spline.cpp
cad/primitive/text.cpp
cad/primitive/dimension.cpp
cad/primitive/dimradial.cpp
cad/primitive/dimdiametric.cpp
cad/primitive/dimlinear.cpp
cad/primitive/dimaligned.cpp
cad/primitive/dimangular.cpp
cad/primitive/lwpolyline.cpp
cad/primitive/beziercurve.cpp
cad/primitive/image.cpp
cad/math/helpermethods.cpp
)

# HEADER FILES
set(lckernel_hdrs
cad/const.h
cad/base/id.h
cad/base/cadentity.h
cad/base/metainfo.h
cad/dochelpers/documentimpl.h
cad/dochelpers/entitycontainer.h
cad/dochelpers/quadtree.h
cad/dochelpers/storagemanagerimpl.h
cad/dochelpers/undomanagerimpl.h
cad/document/document.h
cad/document/storagemanager.h
cad/document/undomanager.h
cad/events/addentityevent.h
cad/events/addlayerevent.h
cad/events/addlinepatternevent.h
cad/events/beginprocessevent.h
cad/events/commitprocessevent.h
cad/events/removeentityevent.h
cad/events/removelayerevent.h
cad/events/removelinepatternevent.h
cad/events/replaceentityevent.h
cad/events/replacelayerevent.h
cad/events/replacelinepatternevent.h
cad/functions/ftrim.h
cad/functions/intersect.h
cad/geometry/geobase.h
cad/geometry/geocoordinate.h
cad/geometry/geoarc.h
cad/geometry/geoarea.h
cad/geometry/geocircle.h
cad/geometry/geoellipse.h
cad/geometry/geospline.h
cad/geometry/geovector.h
cad/geometry/geobezierbase.h
cad/geometry/geobezier.h
cad/geometry/geobeziercubic.h
cad/interface/entitydispatch.h
cad/interface/metatype.h
cad/interface/snapable.h
cad/interface/snapconstrain.h
cad/math/lcmath.h
cad/math/equation.h
cad/math/intersectionhandler.h
cad/meta/color.h
cad/meta/icolor.h
cad/meta/metacolor.h
cad/meta/layer.h
cad/meta/metalinewidth.h
cad/meta/dxflinepattern.h
cad/operations/builder.h
cad/operations/builderops.h
cad/operations/documentoperation.h
cad/operations/trim.h
cad/operations/layerops.h
cad/operations/undoable.h
cad/operations/linepatternops.h
cad/vo/entitycoordinate.h
cad/vo/entitydistance.h
cad/primitive/arc.h
cad/primitive/circle.h
cad/primitive/point.h
cad/primitive/ellipse.h
cad/primitive/line.h
cad/primitive/spline.h
cad/primitive/text.h
cad/primitive/lwpolyline.h
cad/primitive/dimension.h
cad/primitive/dimradial.h
cad/primitive/dimdiametric.h
cad/primitive/dimlinear.h
cad/primitive/dimaligned.h
cad/primitive/dimangular.h
cad/primitive/textconst.h
cad/functions/string_helper.h
cad/primitive/beziercurve.h
cad/primitive/image.h
cad/math/helpermethods.h)

# DEPENDENCIES
find_package(PkgConfig)


# LOG4CXX
pkg_check_modules(LOG4CXX REQUIRED liblog4cxx)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

# Eigen 3
find_package(Eigen3 REQUIRED)
if( CMAKE_COMPILER_IS_GNUCXX)
    include_directories( SYSTEM ${EIGEN3_INCLUDE_DIR})
else ()
    include_directories( ${EIGEN3_INCLUDE_DIR})
endif ()

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)

if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
endif()

# DEBUGGING PATHS
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(lckernel SHARED ${lckernel_srcs} ${lckernel_hdrs} )
target_link_libraries(lckernel opennurbs ${LOG4CXX_LIBRARIES} )
# INSTALLATION
install(TARGETS lckernel DESTINATION lib)
install(FILES ${kernel_hdrs}  DESTINATION include)

