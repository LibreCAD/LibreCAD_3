#!/bin/bash
mkdir -p third_party
cd third_party
mkdir -p gtest_build
cd gtest_build
cmake /usr/src/gtest
make -j2
cd ..
git clone https://github.com/LibreCAD/libdxfrw.git libdxfrw
cd libdxfrw
cmake .
make -j2
cd ../..
mkdir -p build
cd build
cmake -DWITH_COVERAGE=ON ..


make -j2 lcunittest VERBOSE=1
make lccoverage
cd ..
coveralls
