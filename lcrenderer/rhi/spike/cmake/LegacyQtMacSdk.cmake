# The R0 and experimental RHI-only macOS hosts are Metal-only. Qt 6.8/6.9 exports an obsolete consumer
# -framework AGL fallback when the selected Apple SDK no longer contains AGL.
# Qt 6.10 removes this from FindWrapOpenGL.cmake. Keep the provider binaries and
# every other Qt dependency unchanged; this does NOT remove their runtime GL/AGL.
function(lc_spike_sdk_has_agl qt_version result)
    set(available false)
    if(APPLE AND qt_version VERSION_LESS 6.10)
        # A runtime bundle found by find_library is not an SDK link stub.
        set(sdk "${CMAKE_OSX_SYSROOT}")
        if(NOT IS_ABSOLUTE "${sdk}")
            if(NOT sdk)
                set(sdk macosx)
            endif()
            execute_process(COMMAND xcrun --sdk "${sdk}" --show-sdk-path
                OUTPUT_VARIABLE sdk OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ERROR_IS_FATAL ANY)
        endif()
        foreach(stub AGL AGL.tbd Versions/A/AGL Versions/A/AGL.tbd)
            if(EXISTS "${sdk}/System/Library/Frameworks/AGL.framework/${stub}")
                set(available true)
            endif()
        endforeach()
    endif()
    set(${result} ${available} PARENT_SCOPE)
endfunction()

function(lc_spike_legacy_qt_agl qt_version agl_available result)
    set(${result} false PARENT_SCOPE)
    if(NOT APPLE OR qt_version VERSION_LESS 6.8 OR qt_version VERSION_GREATER_EQUAL 6.10
            OR agl_available OR NOT TARGET WrapOpenGL::WrapOpenGL)
        return()
    endif()
    get_target_property(original WrapOpenGL::WrapOpenGL INTERFACE_LINK_LIBRARIES)
    if(NOT original)
        return()
    endif()
    set(filtered "${original}")
    list(FILTER filtered EXCLUDE REGEX "^(-framework AGL|.*/AGL[.]framework(/.*)?)$")
    if(NOT "${filtered}" STREQUAL "${original}")
        set_property(TARGET WrapOpenGL::WrapOpenGL PROPERTY INTERFACE_LINK_LIBRARIES "${filtered}")
        set(${result} true PARENT_SCOPE)
        message(STATUS "R0 Metal host: omitted legacy Qt consumer AGL link item absent from the Apple SDK")
    endif()
endfunction()
