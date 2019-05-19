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
make -j2 lcunittest

if [[ $TRAVIS_TEST_RESULT != 0 ]]; then
  echo "Compilation failed"
  exit 1;
fi

./unittest/lcunittest

make stylecheck > stylecheck.out

if [[ -n $(grep "warning: " stylecheck.out) ]] || [[ -n $(grep "error: " stylecheck.out) ]]; then
  echo "Style checks failed"
  echo ""
  cat stylecheck.out
  exit 1;
else
  echo -e "\033[1;32m\xE2\x9C\x93 passed:\033[0m $1";
  cat stylecheck.out
fi
