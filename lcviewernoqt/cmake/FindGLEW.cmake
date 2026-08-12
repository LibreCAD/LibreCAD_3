# Try to find GLEW
# Once done, this will define :
#   GLEW_FOUND
#   GLEW_INCLUDE_DIR
#   GLEW_LIBRARY
#   GLEW_LIBRARIES
# and, for parity with newer GLEW CMake package configs, the
# GLEW::GLEW imported target.
#
# Homebrew's glew-config.cmake (picked up by find_package(GLEW) in
# Config mode) only defines the GLEW::GLEW/GLEW::glew targets, not
# these legacy variables, so this project needs its own Module-mode
# lookup.

FIND_PACKAGE(PkgConfig QUIET)
PKG_CHECK_MODULES(PC_GLEW QUIET glew)

find_path(GLEW_INCLUDE_DIR
        NAMES
        GL/glew.h

        PATHS
        /usr/include
        /usr/local/include
        /opt/homebrew/include
        $ENV{GLEW_DIR}/include
        ${PC_GLEW_INCLUDEDIR}
)

find_library(GLEW_LIBRARY
        NAMES
        GLEW glew32 glew

        PATHS
        /usr/lib
        /usr/local/lib
        /opt/homebrew/lib
        $ENV{GLEW_DIR}/lib
        ${PC_GLEW_LIBDIR}
        )

set(GLEW_LIBRARIES ${GLEW_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GLEW DEFAULT_MSG GLEW_LIBRARY GLEW_INCLUDE_DIR)
mark_as_advanced(GLEW_INCLUDE_DIR GLEW_LIBRARY)

if(GLEW_FOUND AND NOT TARGET GLEW::GLEW)
    add_library(GLEW::GLEW UNKNOWN IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
        IMPORTED_LOCATION "${GLEW_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}"
    )
endif()
