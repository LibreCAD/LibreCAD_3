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

# For Windows, when building Pacpus use the local log4cxx files, else use the log4cxx files in the pacpus install dir
if(WIN32)
    if(IS_BUILDING_PACPUS)
        SET(LOG4CXX_ROOT "3rd/apache-log4cxx")
    elseif(PACPUS_ROOT)
        SET(LOG4CXX_ROOT "${PACPUS_ROOT}/3rd/apache-log4cxx")
    else()
        SET(LOG4CXX_ROOT "$ENV{PACPUS_ROOT}/3rd/apache-log4cxx")
    endif()
endif()



set(LOG4CXX_INC_LIST
        "/usr/include"
        "/usr/local/include"
        )

set(LOG4CXX_LIB_LIST
        "/usr/lib"
        "/usr/local/lib"
        )

# If LOG4CXX_ROOT is available, set up our hints
if(LOG4CXX_ROOT)
    # Includes
    list(APPEND LOG4CXX_INC_LIST
            "${LOG4CXX_ROOT}/include"
            "${LOG4CXX_ROOT}"
            )

    # Libraries win32
    if(MSVC)
        # Visual Studio 2008
        if(MSVC9)
            list(APPEND LOG4CXX_LIB_LIST "${LOG4CXX_ROOT}/lib/msvc2008")
        endif()
        # Visual Studio 2010
        if(MSVC10)
            list(APPEND LOG4CXX_LIB_LIST "${LOG4CXX_ROOT}/lib/msvc2010")
        endif()
    endif()

    # Libraries all
    list(APPEND LOG4CXX_LIB_LIST "${LOG4CXX_ROOT}/lib")
endif()

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

# Find debug library, if on UNIX this is the same as release
if(WIN32)
    find_library(LOG4CXX_LIBRARY_DEBUG NAMES log4cxx_d HINTS ${LOG4CXX_LIB_LIST})
else()
    set(LOG4CXX_LIBRARY_DEBUG ${LOG4CXX_LIBRARY})
endif()

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

    # Debug Libraries
    if(LOG4CXX_LIBRARY_DEBUG)
        LIST(APPEND LOG4CXX_LIBRARIES debug ${LOG4CXX_LIBRARY_DEBUG})
    endif()

    # Link dirs
    get_filename_component(LOG4CXX_LIBRARY_DIRS ${LOG4CXX_LIBRARY} PATH)
else()
    message(FATAL_ERROR "LOG4CXX library not found")
endif()

# Advanced options for not cluttering the cmake UIs
mark_as_advanced(LOG4CXX_INCLUDE_DIR LOG4CXX_LIBRARY LOG4CXX_LIBRARY_DEBUG)
