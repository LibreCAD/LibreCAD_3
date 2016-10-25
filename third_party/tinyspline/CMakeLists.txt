###############################################################################
### Add different options to this script. The following options are available.
#
# TINYSPLINE_DOUBLE_PRECISION - default: OFF
#   Use double instead of single (float) precision.
#
# TINYSPLINE_DISABLE_CXX11_FEATURES - default: OFF
#   Disable C++11 specific features in C++ interface.
###############################################################################
# TINYSPLINE_DOUBLE_PRECISION
option(TINYSPLINE_DOUBLE_PRECISION "Build TinySpline with double precision" ON)

# TINYSPLINE_DISABLE_CXX11_FEATURES
option(TINYSPLINE_DISABLE_CXX11_FEATURES "Build TinySpline without C++11 features" OFF)



###############################################################################
### Map supported environment variables to CMake variables. The following
### variables will be processed and may overwrite previously defined options
### (only if the option's default value has been selected).
#
# TINYSPLINE_DOUBLE_PRECISION
#   See corresponding option above.
#
# TINYSPLINE_DISABLE_CXX11_FEATURES
#   See corresponding option above.
#
# CMAKE_TOOLCHAIN_FILE
#   The CMake tool chain file for cross-compiling.
#
# SWIG_EXECUTABLE
#   The SWIG executable.
#
# PYTHON_LIBRARY and PYTHON_INCLUDE_DIR
#   The Python environment. Both variables must be available in order to set a
#   custom environment.
###############################################################################
# TINYSPLINE_DOUBLE_PRECISION
if(NOT TINYSPLINE_DOUBLE_PRECISION AND DEFINED ENV{TINYSPLINE_DOUBLE_PRECISION})
  message(STATUS "Using environment variable 'TINYSPLINE_DOUBLE_PRECISION'")
  set(TINYSPLINE_DOUBLE_PRECISION $ENV{TINYSPLINE_DOUBLE_PRECISION})
endif()

# TINYSPLINE_DISABLE_CXX11_FEATURES
if(NOT TINYSPLINE_DISABLE_CXX11_FEATURES AND DEFINED ENV{TINYSPLINE_DISABLE_CXX11_FEATURES})
  message(STATUS "Using environment variable 'TINYSPLINE_DISABLE_CXX11_FEATURES'")
  set(TINYSPLINE_DISABLE_CXX11_FEATURES $ENV{TINYSPLINE_DISABLE_CXX11_FEATURES})
endif()

# CMAKE_TOOLCHAIN_FILE
if(DEFINED ENV{CMAKE_TOOLCHAIN_FILE})
  message(STATUS "Using environment variable 'CMAKE_TOOLCHAIN_FILE'")
  set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
endif()

###############################################################################
### Setup compiler suite. The following variables may be useful in the
### remainder of this script.
#
# CMAKE_SYSTEM_PROCESSOR
#   The name of the target system.
#
# TARGET_SUPPORTS_SHARED_LIBS
#   Does the target support shared libraries?
#
# TINYSPLINE_DEBUG_POSTFIX
#   Postfix of debug builds.
#
# CMAKE_C_FLAGS
#   C flags used by the C library and bindings
#
# CMAKE_CXX_FLAGS
#   C++ flags used by the C++ library and bindings
#
# TINYSPLINE_LIBRARY_C_FLAGS
#   C library specific flags
#
# TINYSPLINE_LIBRARY_CXX_FLAGS
#   C++ library specific flags
#
# TINYSPLINE_BINDING_CXX_FLAGS
#   C++ binding specific flags
#
# TINYSPLINE_CXX_AVAILABLE
#   Is an appropriate C++ compiler available?
###############################################################################
# TARGET_SUPPORTS_SHARED_LIBS
if(NOT TARGET_SUPPORTS_SHARED_LIBS)
  set(TARGET_SUPPORTS_SHARED_LIBS TRUE)
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "avr")
  set(TARGET_SUPPORTS_SHARED_LIBS FALSE)
endif()

# TINYSPLINE_DEBUG_POSTFIX (the default postfix is empty)
set(TINYSPLINE_DEBUG_POSTFIX "")

# TINYSPLINE_DEBUG_POSTFIX
# CMAKE_C_FLAGS
# CMAKE_CXX_FLAGS
# TINYSPLINE_LIBRARY_C_FLAGS
# TINYSPLINE_LIBRARY_CXX_FLAGS
# TINYSPLINE_BINDING_CXX_FLAGS
# TINYSPLINE_CXX_AVAILABLE
set(TINYSPLINE_CXX_AVAILABLE FALSE)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # definitions
  if(TINYSPLINE_DOUBLE_PRECISION)
    add_definitions("-DTINYSPLINE_DOUBLE_PRECISION")
  endif()
  if(TINYSPLINE_DISABLE_CXX11_FEATURES)
    add_definitions("-DTINYSPLINE_DISABLE_CXX11_FEATURES")
  endif()

  # CMAKE_C_FLAGS
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "avr")
    # avr uses the 'inline' keyword in some header files which is not
    # supported by c89
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c89")
  endif()

  # CMAKE_CXX_FLAGS
  if(NOT TINYSPLINE_DISABLE_CXX11_FEATURES)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()

  # TINYSPLINE_LIBRARY_C_FLAGS
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -Werror")
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -Wall")
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -Wextra")
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -Wfloat-equal")
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -pedantic")

  # TINYSPLINE_LIBRARY_CXX_FLAGS 
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} -Werror")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} -Wall")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} -Wextra")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} -Wfloat-equal")

  # TINYSPLINE_BINDING_CXX_FLAGS
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Werror")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wall")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wextra")
  # fixes some warnings of source files generated by swig
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -fno-strict-aliasing")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-missing-field-initializers")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-unused-parameter")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-unused-but-set-variable")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-ignored-qualifiers")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-unused-function")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-sign-compare")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-unused-label")
  
  # TINYSPLINE_CXX_AVAILABLE
  if(TINYSPLINE_DISABLE_CXX11_FEATURES OR
      # http://stackoverflow.com/questions/10984442/how-to-detect-c11-support-of-a-compiler-with-cmake
      CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.7 OR
      CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 4.7)
    set(TINYSPLINE_CXX_AVAILABLE TRUE)
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # definitions
  if(TINYSPLINE_DOUBLE_PRECISION)
    add_definitions("-DTINYSPLINE_DOUBLE_PRECISION")
  endif()
  if(TINYSPLINE_DISABLE_CXX11_FEATURES)
    add_definitions("-DTINYSPLINE_DISABLE_CXX11_FEATURES")
  endif()

  # CMAKE_C_FLAGS
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c89")

  # CMAKE_CXX_FLAGS
  if(NOT TINYSPLINE_DISABLE_CXX11_FEATURES)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  endif()

  # TINYSPLINE_LIBRARY_C_FLAGS
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -Werror")
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -Wall")
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -Wextra")
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -Wfloat-equal")
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} -pedantic")

  # TINYSPLINE_LIBRARY_CXX_FLAGS 
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} -Werror")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} -Wall")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} -Wextra")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} -Wfloat-equal")

  # TINYSPLINE_BINDING_CXX_FLAGS
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Werror")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wall")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wextra")
  # fixes some warnings of source files generated by swig
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-missing-field-initializers")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-unused-parameter")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-ignored-qualifiers")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-unused-function")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-sign-compare")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-deprecated-register")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} -Wno-unused-label")

  # TINYSPLINE_CXX_AVAILABLE
  if(TINYSPLINE_DISABLE_CXX11_FEATURES OR
      # http://stackoverflow.com/questions/10408849/how-can-i-use-c-11-features-in-clang
      CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.3 OR
      CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 3.3)
    set(TINYSPLINE_CXX_AVAILABLE TRUE)
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  # definitions
  if(TINYSPLINE_DOUBLE_PRECISION)
    add_definitions("/DTINYSPLINE_DOUBLE_PRECISION")
  endif()
  if(TINYSPLINE_DISABLE_CXX11_FEATURES)
    add_definitions("/DTINYSPLINE_DISABLE_CXX11_FEATURES")
  endif()

  # TINYSPLINE_LIBRARY_C_FLAGS
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} /Wall")
  set(TINYSPLINE_LIBRARY_C_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS} /WX")

  # TINYSPLINE_LIBRARY_CXX_FLAGS
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} /Wall")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} /WX")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} /wd4514")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} /wd4710")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} /wd4350")
  set(TINYSPLINE_LIBRARY_CXX_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS} /wd4820")

  # TINYSPLINE_BINDING_CXX_FLAGS
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} /Wall")
  # '/WX' causes to many errors in generated source code 
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} /W4")
  # fixes some warnings of source files generated by swig
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} /wd4820")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} /wd4711")
  set(TINYSPLINE_BINDING_CXX_FLAGS "${TINYSPLINE_BINDING_CXX_FLAGS} /wd4702")

  # TINYSPLINE_CXX_AVAILABLE
  if(TINYSPLINE_DISABLE_CXX11_FEATURES OR
      CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 18 OR
      CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 18)
    set(TINYSPLINE_CXX_AVAILABLE TRUE)
  endif()

  # used to automatically create a .def file with all the global symbols for a
  # shared library
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

  # it is common practice for MSVC builds to add a debug postfix
  set(TINYSPLINE_DEBUG_POSTFIX "d")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # TODO: Find appropriate settings for intel

  # definitions
  if(TINYSPLINE_DOUBLE_PRECISION)
    add_definitions("-DTINYSPLINE_DOUBLE_PRECISION")
  endif()
  if(TINYSPLINE_DISABLE_CXX11_FEATURES)
    add_definitions("-DTINYSPLINE_DISABLE_CXX11_FEATURES")
  endif()

  # TINYSPLINE_CXX_AVAILABLE
  if(TINYSPLINE_DISABLE_CXX11_FEATURES OR
      CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 2014 OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 2014)
    set(TINYSPLINE_CXX_AVAILABLE TRUE)
  endif()
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "avr")
  # avr is missing some required headers
  set(TINYSPLINE_CXX_AVAILABLE FALSE)
endif()



###############################################################################
### Create C/C++ libraries. We explicitly set the prefix of the static
### libraries to 'lib' to prevent naming collisions when compiling with MSVC.
###############################################################################
# C library
if(TARGET_SUPPORTS_SHARED_LIBS)
  add_library(tinyspline_shared SHARED tinyspline.c)
  set_target_properties(tinyspline_shared PROPERTIES
    OUTPUT_NAME "tinyspline"
    DEBUG_POSTFIX "${TINYSPLINE_DEBUG_POSTFIX}"
    COMPILE_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS}"
  )
  install(TARGETS tinyspline_shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
endif()
add_library(tinyspline_static STATIC tinyspline.c)
set_target_properties(tinyspline_static PROPERTIES
  OUTPUT_NAME "tinyspline"
  DEBUG_POSTFIX "${TINYSPLINE_DEBUG_POSTFIX}"
  PREFIX "lib"
  COMPILE_FLAGS "${TINYSPLINE_LIBRARY_C_FLAGS}"
)
install(TARGETS tinyspline_static
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(FILES tinyspline.h
  DESTINATION include
)
target_include_directories(tinyspline_static
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# C++ library
if(TINYSPLINE_CXX_AVAILABLE)
  if(TARGET_SUPPORTS_SHARED_LIBS)
    add_library(tinysplinecpp_shared SHARED tinyspline.c tinysplinecpp.cpp)
    set_target_properties(tinysplinecpp_shared PROPERTIES
      OUTPUT_NAME "tinysplinecpp"
      DEBUG_POSTFIX "${TINYSPLINE_DEBUG_POSTFIX}"
      COMPILE_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS}"
    )
    install(TARGETS tinysplinecpp_shared
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
    )
  endif()
  add_library(tinysplinecpp_static STATIC tinyspline.c tinysplinecpp.cpp)
  set_target_properties(tinysplinecpp_static PROPERTIES
    OUTPUT_NAME "tinysplinecpp"
    DEBUG_POSTFIX "${TINYSPLINE_DEBUG_POSTFIX}"
    PREFIX "lib"
    COMPILE_FLAGS "${TINYSPLINE_LIBRARY_CXX_FLAGS}"
  )
  install(TARGETS tinysplinecpp_static
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )
  install(FILES tinysplinecpp.h
    DESTINATION include
  )
  target_include_directories(tinysplinecpp_static
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()

###############################################################################
### Print summary
###############################################################################
message(STATUS
"
******************* TinySpline Configuration Summary *******************
Interface Configuration:
  With double precision  (default: OFF): ${TINYSPLINE_DOUBLE_PRECISION}
  Without C++11 features (default: OFF): ${TINYSPLINE_DISABLE_CXX11_FEATURES}

Compiler Configuration:
  Compiler:          ${CMAKE_CXX_COMPILER}
  C flags:           ${CMAKE_C_FLAGS}
  CXX flags:         ${CMAKE_CXX_FLAGS}
  C library flags:   ${TINYSPLINE_LIBRARY_C_FLAGS}
  C++ library flags: ${TINYSPLINE_LIBRARY_CXX_FLAGS}
  C++ binding flags: ${TINYSPLINE_BINDING_CXX_FLAGS}
  Toolchain file:    ${CMAKE_TOOLCHAIN_FILE}

Available Interfaces:
  C++:		${TINYSPLINE_CXX_AVAILABLE}

  C\#:		TRUE
    Compiler: ${TINYSPLINE_CSHARP_COMPILER}
")
