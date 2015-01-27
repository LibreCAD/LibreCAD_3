CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LCKERNEL)
ADD_DEFINITIONS(-std=c++11)
ADD_DEFINITIONS("-Wall")
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
cad/geometry/geocoordinate.cpp
cad/geometry/geoellipse.cpp
cad/geometry/geospline.cpp
cad/math/lcmath.cpp
cad/math/coordinatesol.cpp
cad/math/quadratic_math.cpp
cad/meta/layer.cpp
cad/meta/icolor.cpp
cad/meta/metalinewidth.cpp
cad/operations/builder.cpp
cad/operations/builderops.cpp
cad/operations/documentoperation.cpp
cad/operations/trim.cpp
cad/operations/layerops.cpp
cad/primitive/arc.cpp
cad/primitive/circle.cpp
cad/primitive/coordinate.cpp
cad/primitive/ellipse.cpp
cad/primitive/line.cpp
cad/primitive/spline.cpp
cad/primitive/text.cpp
cad/primitive/dimension.cpp
cad/primitive/dimradial.cpp
cad/primitive/dimdiametric.cpp
cad/primitive/dimlinear.cpp
cad/primitive/dimaligned.cpp
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
cad/geometry/geoellipse.h
cad/geometry/geospline.h
cad/geometry/geovector.h
cad/interface/entitydispatch.h
cad/interface/entityvisitor.h
cad/interface/metatype.h
cad/interface/snapable.h
cad/math/lcmath.h
cad/math/coordinatesol.h
cad/math/quadratic_math.h
cad/meta/color.h
cad/meta/icolor.h
cad/meta/metacolor.h
cad/meta/layer.h
cad/meta/metalinewidth.h
cad/operations/builder.h
cad/operations/builderops.h
cad/operations/documentoperation.h
cad/operations/trim.h
cad/operations/layerops.h
cad/operations/undoable.h
cad/primitive/arc.h
cad/primitive/circle.h
cad/primitive/coordinate.h
cad/primitive/ellipse.h
cad/primitive/line.h
cad/primitive/spline.h
cad/primitive/text.h
cad/vo/entitycoordinate.h
cad/vo/entitydistance.h
cad/primitive/dimension.h
cad/primitive/dimradial.h
cad/primitive/dimdiametric.h
cad/primitive/dimlinear.h
cad/primitive/dimaligned.h
cad/primitive/textconst.h
cad/functions/str_format.h
)

# DEPENDENCIES

set(Boost_USE_MULTITHREADED ON)
#    set(Boost_ADDITIONAL_VERSIONS "1.55" "1.55.0")
find_package(Boost COMPONENTS thread date_time program_options filesystem system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

#    find_package(maths REQUIRED)
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

add_library(lckernel SHARED ${lckernel_srcs} ${lckernel_hdrs} )
target_link_libraries(lckernel ${LOG4CXX_LIBRARIES} ${Boost_LIBRARIES})
# INSTALLATION
install(TARGETS lckernel DESTINATION lib)
install(FILES ${kernel_hdrs}  DESTINATION include)
