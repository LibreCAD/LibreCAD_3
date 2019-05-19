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
cmake ..


if make -j2 lcunittest; then
  echo "Compilation failed"
  exit 1;
fi

./unittest/lcunittest
