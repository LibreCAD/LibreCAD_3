#!/bin/bash

mkdir -p build
cd build
cmake -DRUN_CLANG_TIDY_PY=/usr/bin/run-clang-tidy-6.0.py -DCHECKS_SOURCE_BRANCH=${TRAVIS_BRANCH} ..

make stylecheck > stylecheck.out

if [[ -n $(grep "warning: " stylecheck.out) ]] || [[ -n $(grep "error: " stylecheck.out) ]]; then
  echo "Style checks failed"
  echo ""
  exit 1;
else
  echo -e "\033[1;32m\xE2\x9C\x93 passed:\033[0m $1";
fi

cat stylecheck.out
