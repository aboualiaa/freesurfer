#!/usr/bin/env bash

set -e

./ci-scripts/restore-mtime.sh
./ci-scripts/install-hpx.sh

sudo xcode-select -r # s /Applications/Xcode.app
git remote add datasrc https://surfer.nmr.mgh.harvard.edu/pub/dist/freesurfer/repo/annex.git
git fetch datasrc
git annex get . || true

export PATH="$(pwd)/hpx-install/:$PATH"
mkdir -p cmake-build-debug && cd ./cmake-build-debug
sed -i '' '/CMAKE_CXX_COMPILER:FILEPATH/d' ./CMakeCache.txt || true # remove compiler cache entry just in case travis updated xcode
sed -i '' '/\/\/CXX compiler/d' ./CMakeCache.txt || true
sed -i '' '/CMAKE_C_COMPILER:FILEPATH/d' ./CMakeCache.txt || true # remove compiler cache entry just in case travis updated xcode
sed -i '' '/\/\/C compiler/d' ./CMakeCache.txt || true
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGEMS_BUILD_MATLAB=OFF ..
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGEMS_BUILD_MATLAB=OFF .. # again so that AppleClang finds OpenMP
