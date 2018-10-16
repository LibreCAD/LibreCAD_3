#
# Copyright 2013 Produban
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Tries to find Log4cxx Client headers and libraries.
#
# Usage of this module as follows:
#
#  find_package(APR)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  APR_ROOT_DIR  Set this variable to the root installation of Apache Portable
#                Runtime/Util Library if the module has problems finding the
#                proper installation path.
#
# Variables defined by this module:
#
#  APR_FOUND              System has Apache Portable Runtime libs/headers
#  APR_LIBRARIES          The Apache Portable Runtime libraries
#  APR_INCLUDE_DIRS       The location of Apache Portable Runtime headers

message ("\nLooking for Apache Portable Runtime/Util headers and libraries")

find_package(PkgConfig)
pkg_search_module(APR "apr-1 apr-util-1")

if(NOT APR_FOUND AND NOT PKGCONFIG_FOUND)
    pkg_check_modules(PC_APR apr-1)
    set(APR_DEFINITIONS ${PC_APR_CFLAGS_OTHER})

    find_path(APR_INCLUDE_DIR
            NAMES
            apr.h
            PATHS
            ${APR_ROOT_DIR}/include
            ${PC_APR_INCLUDEDIR}
            ${PC_APR_INCLUDE_DIRS}
            PATH_SUFFIXES
            apr-1
            )

    find_library(APR_LIBRARY
            NAMES
            apr-1
            PATHS
            ${APR_ROOT_DIR}/lib
            ${PC_APR_LIBDIR}
            ${PC_APR_LIBRARY_DIRS}
            )

    pkg_check_modules(PC_APRUTIL apr-util-1)
    set(APRUTIL_DEFINITIONS ${PC_APRUTIL_CFLAGS_OTHER})

    find_path(APRUTIL_INCLUDE_DIR
            NAMES
            apu.h
            HINTS
            ${APR_ROOT_DIR}/include
            ${PC_APRUTIL_INCLUDEDIR}
            ${PC_APRUTIL_INCLUDE_DIRS}
            PATH_SUFFIXES
            apr-1
            )

    find_library(APRUTIL_LIBRARY
            NAMES
            aprutil-1 apr-util-1
            HINTS
            ${APR_ROOT_DIR}/lib
            ${PC_APRUTIL_LIBDIR}
            ${PC_APRUTIL_LIBRARY_DIRS}
            )

    if(APR_FOUND AND APRUTIL_FOUND)
        set(APR_INCLUDE_DIRS ${APR_INCLUDE_DIR} ${APRUTIL_INCLUDE_DIR})
        set(APR_LIBRARIES ${APR_LIBRARY} ${APRUTIL_LIBRARY})
    endif()
endif()

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set APR_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(APR
        DEFAULT_MSG
        APR_LIBRARY
        APR_INCLUDE_DIR
        )

# handle the QUIETLY and REQUIRED arguments and set APR_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(APRUTIL
        DEFAULT_MSG
        APRUTIL_LIBRARY
        APRUTIL_INCLUDE_DIR
        )

mark_as_advanced(
        APR_INCLUDE_DIR
        APR_LIBRARY
        APRUTIL_INCLUDE_DIR
        APRUTIL_LIBRARY
)