CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)
PROJECT (LC)

#option(ENABLE_MY_ASSERT "Turn on NDEBUG checks" OFF)
#if(NDEBUG)
#  add_definitions(-DNDEBUG=1)
#else()
#  add_definitions(-DNDEBUG=0)
#endif()

option(WITH_UNITTESTS "Build unit tests" ON)
option(WITH_LCDXFDWG "Build dxf/dwg support" ON)
option(WITH_QTUI "Build Qt graphical user interface" ON)
option(WINDOWS_MSYS2 "Compile on Windows system" OFF)


if(WINDOWS_MSYS2)
    add_definitions(-DDISABLE_OPENNURBS=1)
    add_definitions(-DWINDOWS_MSYS2=1)
    set(G_EXTRA_LIBS "stdc++")
else()
    add_subdirectory("third_party/opennurbs_20130711")
endif()

ADD_SUBDIRECTORY("lckernel")
ADD_SUBDIRECTORY("lcDXFDWG")

add_definitions(-DUSE_lcDXFDWG=1)

ADD_SUBDIRECTORY("lcviewernoqt")
ADD_SUBDIRECTORY("lcviewerqt")
ADD_SUBDIRECTORY("lcadluascript")
if(WITH_QTUI)
    add_definitions(-DWITH_QTUI=${WITH_QTUI})
    ADD_SUBDIRECTORY("lcUI")
endif(WITH_QTUI)
ADD_SUBDIRECTORY("lcUILua")

if(WITH_UNITTESTS)
    ADD_SUBDIRECTORY("unittest")
endif()
ADD_SUBDIRECTORY("luacmdinterface")