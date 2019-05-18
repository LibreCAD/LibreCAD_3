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
cmake -DRUN_CLANG_TIDY_PY=/usr/bin/run-clang-tidy-6.0.py -DCHECKS_SOURCE_BRANCH=${TRAVIS_BRANCH} ..
make -j2

if [[ $TRAVIS_TEST_RESULT != 0 ]]; then
  echo "Compilation failed"
  cat stylecheck.out
  exit 1;
fi

./unittest/lcunittest
