#!/bin/bash

# Little script to clean out the generated and makefiles
rm -rf `find . -type f -name Makefile`
rm -rf `find . -type d -name generated`
rm -rf demo.app
rm -rf unix
find . -name "*.orig" -exec rm {} \;