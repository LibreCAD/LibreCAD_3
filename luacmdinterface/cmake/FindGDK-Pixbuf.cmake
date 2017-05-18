# Try to find gdk-pixbuf 2.0
# Once done, this will define :
#   GDK-PIXBUF_FOUND
#   GDK-PIXBUF_INCLUDE_DIRS
#   GDK-PIXBUF_LIBRARIES

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_GTK-PIXBUF QUIET gdk-pixbuf-2.0)

# Main include dir
find_path(GDK-PIXBUF_INCLUDE_DIRS
        NAMES
        gdk-pixbuf/gdk-pixbuf.h

        PATH_SUFFIXES
        gdk-pixbuf-2.0

        PATHS
        /usr/include
        /usr/local/include
        $ENV{GDK-PIXBUF_DIR}/include
        ${PC_GTK_PIXBUF_includedir}
)

# Finally the library itself
find_library(GDK-PIXBUF_LIBRARIES
        NAMES
        gdk_pixbuf-2.0

        PATHS
        /usr/lib
        /usr/local/lib
        $ENV{GDK-PIXBUF_DIR}/lib
        ${PC_GTK_PIXBUF_libdir}
        )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(GDK-PIXBUF DEFAULT_MSG GDK-PIXBUF_LIBRARIES GDK-PIXBUF_INCLUDE_DIRS)
mark_as_advanced(GDK-PIXBUF_INCLUDE_DIRS GDK-PIXBUF_LIBRARIES)