#!/bin/bash
OUTPUT_DIRECTORY=doxygen/doc
#doxygen is main module
# Phase 6 PR-6.2 — added `lcadpythonscript` + `lcscripting`.  Order-
# matters caveat still applies (per the comment below), but neither
# new module cycles into the existing ones — lcadpythonscript links
# lckernel + lcscripting, lcscripting has no upstream dep.
MODULES=( lckernel persistence lcviewernoqt lcscripting lcadluascript lcadpythonscript lcUI luacmdinterface unittest )
#here order matters, if cyclic 2 run

#This file generate Doxygen documentation
#Maybe we should execute the following commands with CMake.

createDirectory(){
    if [ ! -r "$1" ]; then
        echo "Creating directory $1"
        mkdir "$1"
    fi
}

createDirectory $OUTPUT_DIRECTORY
for module in ${MODULES[@]}
do
    createDirectory $OUTPUT_DIRECTORY/$module
    (cat $OUTPUT_DIRECTORY/../librecad.dox; echo "INPUT=$module"; echo "OUTPUT_DIRECTORY=$OUTPUT_DIRECTORY/$module"; echo "PROJECT_BRIEF=LibreCAD_3" ; echo "PROJECT_NAME=$module" ; cat doxygen/$module.cfg) | doxygen -
done
createDirectory $OUTPUT_DIRECTORY/main
(cat $OUTPUT_DIRECTORY/../librecad.dox; echo "INPUT=doxygen"; echo "OUTPUT_DIRECTORY=$OUTPUT_DIRECTORY/main"; cat doxygen/main.cfg) | doxygen -
