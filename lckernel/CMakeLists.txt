CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)
PROJECT (LCKERNEL)
ADD_DEFINITIONS(-std=c++11)

# SOURCE FILES
set(lckernel_srcs
cad/base/cadentity.cpp
cad/base/id.cpp
cad/base/metainfo.cpp
cad/dochelpers/documentimpl.cpp
cad/dochelpers/entitycontainer.cpp
cad/dochelpers/quadtree.cpp
cad/dochelpers/storagemanagerimpl.cpp
cad/dochelpers/undomanagerimpl.cpp
cad/document/document.cpp
cad/functions/ftrim.cpp
cad/functions/intersect.cpp
cad/geometry/geoarc.cpp
cad/geometry/geocircle.cpp
cad/geometry/geodimaligned.cpp
cad/geometry/geodimangular.cpp
cad/geometry/geodimdiametric.cpp
cad/geometry/geodimension.cpp
cad/geometry/geodimlinear.cpp
cad/geometry/geodimradial.cpp
cad/geometry/geoellipse.cpp
cad/geometry/geomtext.cpp
cad/geometry/geospline.cpp
cad/geometry/geotext.cpp
cad/math/lcmath.cpp
cad/math/coordinatesol.cpp
cad/math/quadratic_math.cpp
cad/meta/layer.cpp
cad/meta/linewidth.cpp
cad/meta/pen.cpp
cad/operations/builder.cpp
cad/operations/builderops.cpp
cad/operations/documentoperation.cpp
cad/operations/trim.cpp
cad/primitive/arc.cpp
cad/primitive/circle.cpp
cad/primitive/coordinate.cpp
cad/primitive/dimaligned.cpp
cad/primitive/dimangular.cpp
cad/primitive/dimdiametric.cpp
cad/primitive/dimension.cpp
cad/primitive/dimlinear.cpp
cad/primitive/dimradial.cpp
cad/primitive/ellipse.cpp
cad/primitive/line.cpp
cad/primitive/mtext.cpp
cad/primitive/spline.cpp
cad/primitive/text.cpp
)

# HEADER FILES
set(lckernel_hdrs cad/base/cadentity.h
cad/base/id.h
cad/base/metainfo.h
cad/dochelpers/documentimpl.h
cad/dochelpers/entitycontainer.h
cad/dochelpers/quadtree.h
cad/dochelpers/storagemanagerimpl.h
cad/dochelpers/undomanagerimpl.h
cad/document/document.h
cad/document/selectionmanager.h
cad/document/storagemanager.h
cad/document/undomanager.h
cad/events/addentityevent.h
cad/events/addlayerevent.h
cad/events/beginprocessevent.h
cad/events/commitprocessevent.h
cad/events/removeentityevent.h
cad/events/removelayerevent.h
cad/events/replaceentityevent.h
cad/events/replacelayerevent.h
cad/functions/ftrim.h
cad/functions/intersect.h
cad/geometry/geoarc.h
cad/geometry/geoarea.h
cad/geometry/geocircle.h
cad/geometry/geocoordinate.h
cad/geometry/geodimaligned.h
cad/geometry/geodimangular.h
cad/geometry/geodimdiametric.h
cad/geometry/geodimension.h
cad/geometry/geodimlinear.h
cad/geometry/geodimradial.h
cad/geometry/geoellipse.h
cad/geometry/geomtext.h
cad/geometry/geospline.h
cad/geometry/geotext.h
cad/geometry/geovector.h
cad/interface/entitydispatch.h
cad/interface/entityvisitor.h
cad/interface/metatype.h
cad/interface/snapable.h
cad/interface/variantable.h
cad/math/lcmath.h
cad/math/coordinatesol.h
cad/math/quadratic_math.h
cad/meta/color.h
cad/meta/layer.h
cad/meta/linewidth.h
cad/meta/pen.h
cad/operations/builder.h
cad/operations/builderops.h
cad/operations/documentoperation.h
cad/operations/trim.h
cad/operations/undoable.h
cad/primitive/arc.h
cad/primitive/circle.h
cad/primitive/coordinate.h
cad/primitive/dimaligned.h
cad/primitive/dimangular.h
cad/primitive/dimdiametric.h
cad/primitive/dimension.h
cad/primitive/dimlinear.h
cad/primitive/dimradial.h
cad/primitive/ellipse.h
cad/primitive/line.h
cad/primitive/mtext.h
cad/primitive/spline.h
cad/primitive/text.h
cad/vo/entitycoordinate.h
cad/vo/entitydistance.h
)

# DEPENDENCIES

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig)
pkg_check_modules(LOG4CXX REQUIRED liblog4cxx)
include_directories(${LOG4CXX_INCLUDE_DIRS})
link_directories(${LOG4CXX_LIBRARY_DIRS})

# BUILDING CONFIG
# SEPERATE BUILDING FLAG
set(SEPARATE_BUILD OFF)
 
if (SEPARATE_BUILD STREQUAL "ON")
include_directories("${CMAKE_SOURCE_DIR}")
else() 
include_directories("${CMAKE_SOURCE_DIR}/lckernel")
endif()
 
# DEBUGGING PATHS 
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_library(lckernel SHARED ${lckernel_srcs} ${lckernel_hdrs})
target_link_libraries(lckernel ${LOG4CXX_LIBRARIES})
# INSTALLATION
install(TARGETS lckernel DESTINATION lib)
install(FILES ${kernel_hdrs}  DESTINATION include)
