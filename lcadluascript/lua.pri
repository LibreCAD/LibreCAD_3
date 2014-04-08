

unix {
    defineTest( checkLuaIncDir ) {

        luaIncDir = $${1}

        exists( $${luaIncDir}/lua.hpp ) {
            return( true )
        }

        return(false)
    }

    defineTest( checkLuaLibDir ) {

        luaLibDir = $${1}
        exists( $${luaLibDir}/liblua* ) {
            return( true )
        }

        return( false )
    }

    defineTest( checkLuaLib ) {

        luaLib = $${1}

        exists( $${LUA_LIBDIR}/liblua$${luaLib}.* ) {
            return( true )
        }

        return( false )
    }

    defineTest( checkLuaLibs ) {

        luaLibs = $${ARGS}

        for( luaLib, luaLibs ) {
            checkLuaLib( $${luaLib} ) {
                return( true )
            } else {
                error( Required Lua library $${luaLib} not found )
            }
        }

        return( false )
    }

    defineReplace( findLuaDirIn ) {

        luaDirs = $${ARGS}

        for( luaDir, luaDirs ) {
            checkLuaIncDir( $${luaDir}/include ) : checkLuaLibDir( $${luaDir}/lib ) {
                return( $${luaDir} )
            }
        }

        return( )
    }

    isEmpty( LUA_DIR ) {

        LUA_DIR = $$findLuaDirIn( /usr /usr/local /usr/pkg /opt/local )
        isEmpty( LUA_DIR ) {
            error( Lua installation not found )
        } else {
            message( Found Lua in $${LUA_DIR} )
        }

    }

    checkLuaIncDir( $${LUA_DIR}/include ) {
        LUA_INCDIR = $${LUA_DIR}/include
        message( Using Lua includes from $${LUA_INCDIR} )
    } else {
        error( $${LUA_DIR} does not contain a Lua installation )
    }

    isEmpty( LUA_LIBS ) {
        checkLuaLibDir( $${LUA_DIR}/lib ) {
            LUA_LIBDIR = $${LUA_DIR}/lib
            LIBS += -L$${LUA_LIBDIR}
            LIBS += -llua -lm
        } else {
            error( Lua libraries not installed in $${LUA_DIR}/lib )
        }

        checkLuaLibs( $${LUA_LIBS} ) {
            # NOTREACHED
            error( Required Lua libraries not found )
        }
    }


    INCLUDEPATH += $${LUA_INCDIR}
    HEADERS += $${LUA_INCDIR}

}

win32 {

    exists( "$$(LUA_DIR)" ) {			# Is it set in the environment?
        LUA_DIR = "$$(LUA_DIR)"		# Yes, use
    } else:isEmpty( LUA_DIR ) {		# Is it set in custom.pro?
        LUA_DIR = "/lua/lua0"	# No, hardcode
    }

    !exists( "$${LUA_DIR}/lua-5.3/lua.hpp" ) {
        error( "Can not find Lua installation in $${LUA_DIR}" )
    }

    LUA_INCDIR = "$${LUA_DIR}"

    INCLUDEPATH += "$${LUA_INCDIR}"
    HEADERS += "$${LUA_INCDIR}"

}

