cmake_minimum_required(VERSION 2.8.11)
project(LCKERNEL)

add_definitions("-Wall")

#Include TinySpline
add_definitions("-DTINYSPLINE_DOUBLE_PRECISION")
set(CMAKE_EXPORT_COMPILE_COMMANDS OFF)
add_subdirectory("tinyspline")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-std=c++14)

message("***** LibreCAD kernel *****")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# SOURCE FILES
set(lckernel_srcs
version.cpp
cad/storage/settings.cpp
cad/base/cadentity.cpp
cad/base/id.cpp
cad/base/metainfo.cpp
cad/storage/documentimpl.cpp
cad/storage/entitycontainer.cpp
cad/storage/quadtree.cpp
cad/storage/storagemanagerimpl.cpp
cad/storage/undomanagerimpl.cpp
cad/storage/document.cpp
cad/math/intersect.cpp
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
cad/operations/entitybuilder.cpp
cad/operations/entityops.cpp
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
cad/primitive/image.cpp
cad/primitive/insert.cpp
cad/math/helpermethods.cpp
cad/meta/block.cpp
cad/builders/line.cpp
cad/builders/arc.cpp
cad/builders/circle.cpp
cad/builders/dimaligned.cpp
cad/builders/dimangular.cpp
cad/builders/dimension.cpp
cad/builders/dimdiametric.cpp
cad/builders/dimlinear.cpp
cad/builders/ellipse.cpp
cad/builders/point.cpp
cad/builders/insert.cpp
cad/builders/layer.cpp
cad/builders/linepattern.cpp
cad/meta/customentitystorage.cpp
cad/operations/blockops.cpp
cad/operations/builder.cpp
cad/primitive/customentity.cpp
cad/interface/snapconstrain.cpp
cad/storage/documentlist.cpp
)

# HEADER FILES
set(lckernel_hdrs
version.h
cad/const.h
cad/storage/settings.h
cad/base/id.h
cad/base/cadentity.h
cad/base/metainfo.h
cad/storage/documentimpl.h
cad/storage/entitycontainer.h
cad/storage/quadtree.h
cad/storage/storagemanagerimpl.h
cad/storage/undomanagerimpl.h
cad/storage/document.h
cad/storage/storagemanager.h
cad/storage/undomanager.h
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
cad/math/intersect.h
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
cad/operations/entitybuilder.h
cad/operations/entityops.h
cad/operations/documentoperation.h
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
cad/tools/string_helper.h
cad/primitive/image.h
cad/math/helpermethods.h
version.h
cad/meta/block.h
cad/builders/cadentity.h
cad/builders/line.h
cad/builders/arc.h
cad/builders/circle.h
cad/builders/dimaligned.h
cad/builders/dimangular.h
cad/builders/dimension.h
cad/builders/dimdiametric.h
cad/builders/dimlinear.h
cad/builders/ellipse.cpp
cad/builders/point.h
cad/primitive/insert.h
cad/builders/insert.h
cad/builders/layer.h
cad/builders/linepattern.h
cad/meta/customentitystorage.h
cad/operations/blockops.h
cad/operations/builder.h
cad/base/visitor.h
cad/primitive/customentity.h
cad/events/newwaitingcustomentityevent.h
cad/storage/documentlist.h
cad/interface/unmanageddraggable.h
cad/interface/tangentable.h)

# LOG4CXX
find_package(Log4CXX 1.10 REQUIRED)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

include_directories("${CMAKE_SOURCE_DIR}/lckernel")
include_directories("${CMAKE_SOURCE_DIR}/lcviewernoqt")

# Eigen 3
find_package(Eigen3 REQUIRED)
if(CMAKE_COMPILER_IS_GNUCXX)
    include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})
else()
    include_directories(${EIGEN3_INCLUDE_DIR})
endif()

# BUILDING CONFIG
# SEPARATE BUILDING FLAG
set(SEPARATE_BUILD OFF)

if (SEPARATE_BUILD STREQUAL "ON")
    include_directories("${CMAKE_SOURCE_DIR}")
    include_directories("${CMAKE_SOURCE_DIR}/../third_party")
else()
    include_directories("${CMAKE_SOURCE_DIR}/lckernel")
    include_directories("${CMAKE_SOURCE_DIR}/third_party")
endif()

#Create version.cpp
add_custom_command(
        PRE_BUILD
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
        COMMAND ${CMAKE_COMMAND} -DDIR=${CMAKE_CURRENT_SOURCE_DIR} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetBuildInfo.cmake"
        DEPENDS always_rebuild
)

# Anything that depends on this will always be rebuilt.
add_custom_command(
        OUTPUT always_rebuild
        COMMAND ${CMAKE_COMMAND} -E echo
)

add_library(lckernel SHARED ${lckernel_srcs} ${lckernel_hdrs})
target_link_libraries(lckernel ${LOG4CXX_LIBRARIES} ${APR_LIBRARIES} ${G_EXTRA_LIBS} tinysplinecpp_shared)

# INSTALLATION
install(TARGETS lckernel DESTINATION lib)
install(FILES ${kernel_hdrs}  DESTINATION include)

