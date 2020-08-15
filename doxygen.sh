#!/bin/bash
OUTPUT_DIRECTORY=doc
MODULES=( lcadluascript persistence lckernel lcUI lcviewernoqt luacmdinterface unittest )
EXCLUDE_PATTERNS="*/nano-signal-slot/* */tinyspline/* */libopencad/*"
#kaguya is outside MODULES

#This file generate Doxygen documentation
#Maybe we should execute the following commands with CMake.

if [ ! -r "$OUTPUT_DIRECTORY" ]; then
    echo "Creating directory $OUTPUT_DIRECTORY"
    mkdir "$OUTPUT_DIRECTORY"
fi

for module in ${MODULES[@]}
do
    (cat librecad.dox; echo "INPUT=$module"; echo "OUTPUT_DIRECTORY=doc/$module"; echo "EXCLUDE_PATTERNS = $EXCLUDE_PATTERNS") | doxygen -
done
