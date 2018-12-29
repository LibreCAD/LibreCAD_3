# Try to find gdk 3.0
# Once done, this will define :
#   GDK_FOUND
#   GDK_INCLUDE_DIRS
#   GDK_LIBRARIES

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_GDK QUIET gdk-3.0)

# Main include dir
find_path(GDK_INCLUDE_DIRS
        NAMES
        gdk/gdk.h

        PATH_SUFFIXES
        gtk-3.0

        PATHS
        /usr/include
        /usr/local/include
        $ENV{GDK_DIR}/include
        ${PC_GDK_includedir}
)

# Finally the library itself
find_library(GDK_LIBRARIES
        NAMES
        gdk-3

        PATHS
        /usr/lib
        /usr/local/lib
        $ENV{GDK-PIXBUF_DIR}/lib
        ${PC_GDK_libdir}
        )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GDK DEFAULT_MSG GDK_LIBRARIES GDK_INCLUDE_DIRS)
mark_as_advanced(GDK_INCLUDE_DIRS GDK_LIBRARIES)