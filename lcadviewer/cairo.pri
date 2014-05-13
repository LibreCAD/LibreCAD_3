

unix {
    defineTest( checkCairoIncDir ) {

        cairoIncDir = $${1}

        exists( $${cairoIncDir}/cairo.h ) {
            return( true )
        }

        return(false)
    }

    defineTest( checkCairoLibDir ) {

        cairoLibDir = $${1}
        exists( $${cairoLibDir}/libcairo* ) {
            return( true )
        }

        return( false )
    }

    defineTest( checkCairoLib ) {

        cairoLib = $${1}

        exists( $${CAIRO_LIBDIR}/libcairo$${cairoLib}.* ) {
            return( true )
        }

        return( false )
    }

    defineTest( checkCairoLibs ) {

        cairoLibs = $${ARGS}

        for( cairoLib, cairoLibs ) {
            checkCairoLib( $${cairoLib} ) {
                return( true )
            } else {
                error( Required Cairo library $${cairoLib} not found )
            }
        }

        return( false )
    }

    defineReplace( findCairoDirIn ) {

        cairoDirs = $${ARGS}

        for( cairoDir, cairoDirs ) {
            checkCairoIncDir( $${cairoDir}/include/cairo ) : checkCairoLibDir( $${cairoDir}/lib ) {
                return( $${cairoDir} )
            }
        }

        return( )
    }

    isEmpty( CAIRO_DIR ) {

        CAIRO_DIR = $$findCairoDirIn( /usr /usr/local /usr/pkg /opt/local )
        isEmpty( CAIRO_DIR ) {
            error( Cairo installation not found )
        } else {
            message( Found Cairo in $${CAIRO_DIR} )
        }

    }

    checkCairoIncDir( $${CAIRO_DIR}/include/cairo  ) {
        CAIRO_INCDIR = $${CAIRO_DIR}/include/cairo
        message( Using Cairo includes from $${CAIRO_INCDIR} )
    } else {
        error( $${CAIRO_DIR} does not contain a Cairo installation )
    }

    isEmpty( CAIRO_LIBS ) {
        checkCairoLibDir( $${CAIRO_DIR}/lib ) {
            CAIRO_LIBDIR = $${CAIRO_DIR}/lib
            LIBS += -L$${CAIRO_LIBDIR}
            LIBS += -lcairo
         message( Using Cairo libs from $${CAIRO_LIBDIR} )
        } else {
            error( Cairo libraries not installed in $${CAIRO_DIR}/lib )
        }

        checkCairoLibs( $${CAIRO_LIBS} ) {
            error( Required Cairo libraries not found )
        }
    }

    INCLUDEPATH += $${CAIRO_INCDIR}
    HEADERS += $${CAIRO_INCDIR}
    DEPENDPATH += $${CAIRO_INCDIR}
message(libs : $${LIBS})

}

win32 {

    exists( "$$(CAIRO_DIR)" ) {			# Is it set in the environment?
        CAIRO_DIR = "$$(CAIRO_DIR)"		# Yes, use
    } else:isEmpty( CAIRO_DIR ) {		# Is it set in custom.pro?
        CAIRO_DIR = "/cairo/cairo0"	# No, hardcode
    }

    !exists( "$${CAIRO_DIR}/cairo-5.3/cairo.hpp" ) {
        error( "Can not find Cairo installation in $${CAIRO_DIR}" )
    }

    CAIRO_INCDIR = "$${CAIRO_DIR}"

    INCLUDEPATH += "$${CAIRO_INCDIR}"
    HEADERS += "$${CAIRO_INCDIR}"

}

