################################################################################
#
# CMake script for finding Log4cxx.
# The default CMake search process is used to locate files.
#
# This script creates the following variables:
#  LOG4CXX_FOUND: Boolean that indicates if the package was found
#  LOG4CXX_INCLUDE_DIRS: Paths to the necessary header files
#  LOG4CXX_LIBRARIES: Package libraries
#  LOG4CXX_LIBRARY_DIRS: Path to package libraries
#
################################################################################

include(FindPackageHandleStandardArgs)
find_package(PkgConfig)

set(LOG4CXX_INC_LIST
        "/usr/include"
        "/usr/local/include"
        "/mingw32/include"
        "/mingw64/include"
)

set(LOG4CXX_LIB_LIST
        "/usr/lib"
        "/usr/local/lib"
        "/mingw32/lib"
        "/mingw64/lib"
)

# If LOG4CXX_ROOT is available, set up our hints
if(LOG4CXX_ROOT)
    # Includes
    list(APPEND LOG4CXX_INC_LIST
            "${LOG4CXX_ROOT}/src/main/include"
            "${LOG4CXX_ROOT}/include"
            "${LOG4CXX_ROOT}"
    )

    # Libraries all
    list(APPEND LOG4CXX_LIB_LIST "${LOG4CXX_ROOT}/lib")
endif()

#Search lib on pkg-config
pkg_search_module(PC_LOG4CXX "liblog4cxx")

if(PC_LOG4CXX_FOUND)
    list(APPEND LOG4CXX_INC_LIST
            ${PC_LOG4CXX_INCLUDEDIR}
    )

    list(APPEND LOG4CXX_LIB_LIST
            ${PC_LOG4CXX_LIBDIR}
    )
endif(PC_LOG4CXX_FOUND)

# Find headers
find_path(
        LOG4CXX_INCLUDE_DIR
        NAMES
        log4cxx/log4cxx.h
        HINTS
        ${LOG4CXX_INC_LIST}
)

# Find release library
find_library(
        LOG4CXX_LIBRARY
        NAMES
        log4cxx
        HINTS
        ${LOG4CXX_LIB_LIST}
)

# Set LOG4CXX_FOUND honoring the QUIET and REQUIRED arguments
find_package_handle_standard_args(LOG4CXX DEFAULT_MSG LOG4CXX_LIBRARY LOG4CXX_INCLUDE_DIR)

# Output variables
if(LOG4CXX_FOUND)
    # Include dirs
    set(LOG4CXX_INCLUDE_DIRS ${LOG4CXX_INCLUDE_DIR})

    # Release Libraries
    if(LOG4CXX_LIBRARY)
        LIST(APPEND LOG4CXX_LIBRARIES optimized ${LOG4CXX_LIBRARY})
    endif()

    # Link dirs
    get_filename_component(LOG4CXX_LIBRARY_DIRS ${LOG4CXX_LIBRARY} PATH)

    message(STATUS "Found log4cxx include dirs: ${LOG4CXX_INCLUDE_DIRS}")
    message(STATUS "Found log4cxx library: ${LOG4CXX_LIBRARY}")
else()
    message(FATAL_ERROR "LOG4CXX library not found")
endif()

# Advanced options for not cluttering the cmake UIs
mark_as_advanced(LOG4CXX_INCLUDE_DIR LOG4CXX_LIBRARY LOG4CXX_LIBRARY_DEBUG)
