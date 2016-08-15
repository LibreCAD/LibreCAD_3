#!/bin/bash

#This file generate Doxygen documentation
#Maybe we should execute the following commands with CMake.

(cat librecad.dox; echo "INPUT=lcadluascript"; echo "OUTPUT_DIRECTORY=doc/lcadluascript") | doxygen -
(cat librecad.dox; echo "INPUT=lcDXFDWG"; echo "OUTPUT_DIRECTORY=doc/lcDXFDWG") | doxygen -
(cat librecad.dox; echo "INPUT=lckernel"; echo "OUTPUT_DIRECTORY=doc/lckernel") | doxygen -
(cat librecad.dox; echo "INPUT=lcUI"; echo "OUTPUT_DIRECTORY=doc/lcUI") | doxygen -
(cat librecad.dox; echo "INPUT=lcviewernoqt"; echo "OUTPUT_DIRECTORY=doc/lcviewernoqt") | doxygen -
(cat librecad.dox; echo "INPUT=lcviewerqt"; echo "OUTPUT_DIRECTORY=doc/lcviewerqt") | doxygen -
(cat librecad.dox; echo "INPUT=luacmdinterface"; echo "OUTPUT_DIRECTORY=doc/luacmdinterface") | doxygen -
(cat librecad.dox; echo "INPUT=unittest"; echo "OUTPUT_DIRECTORY=doc/unittest") | doxygen -