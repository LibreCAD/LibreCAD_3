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
cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON .
make -j2
cd ../..
mkdir -p build
cd build
cmake -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DWITH_COVERAGE=ON ..


make -j2 lcunittest VERBOSE=1
make lccoverage
cd ..
coveralls -e third_party -e unittest -e lckernel/tinyspline -e lckernel/nano-signal-slot -e build/CMakeFiles 
