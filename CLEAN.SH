#!/bin/sh

# Little script to clean out the generated and makefiles
rm -rf `find . -type f -name Makefile`
rm -rf `find . -type d -name generated`
rm -rf `find . -type f -name *.dylib`
rm -rf `find . -type f -name *.*~`
rm -rf `find . -type f -name *.so`
rm -rf `find . -type f -name CMakeCache.txt`
rm -rf demo.app
rm -rf unix
rm -rf doc
rm -rf librecad
find . -name "*.orig" -exec rm {} \;
find . -name ".DS_Store" -exec rm {} \;
