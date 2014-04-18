#!/bin/bash

( cat kerneldev.dox ; echo "INPUT=lckernel" | echo "OUTPUT_DIRECTORY=doc/lckernel" ) | doxygen -
( cat kerneldev.dox ; echo "INPUT=lcadviewer" | echo "OUTPUT_DIRECTORY=doc/lcadviewer" ) | doxygen -
( cat kerneldev.dox ; echo "INPUT=lcadluascript" | echo "OUTPUT_DIRECTORY=doc/lcadluascript" ) | doxygen -
( cat kerneldev.dox ; echo "INPUT=demo" | echo "OUTPUT_DIRECTORY=doc/demo" ) | doxygen -
