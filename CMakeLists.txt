CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT (LC)

option(WITH_UNITTESTS "Build unit tests" ON)
option(WITH_LCDXFDWG "Build dxf/dwg support" OFF)

ADD_SUBDIRECTORY("lckernel")

if(WITH_LCDXFDWG)
    ADD_SUBDIRECTORY("lcDXFDWG")
    add_definitions(-DUSE_lcDXFDWG=1)
endif()

ADD_SUBDIRECTORY("lcviewernoqt")
ADD_SUBDIRECTORY("lcviewerqt")
ADD_SUBDIRECTORY("lcadluascript")
ADD_SUBDIRECTORY("lcUI")
ADD_SUBDIRECTORY("luacmdinterface")
#ADD_SUBDIRECTORY("playground")

if(WITH_UNITTESTS)
    ADD_SUBDIRECTORY("unittest")
endif()
