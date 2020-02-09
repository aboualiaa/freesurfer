#!/bin/bash

sudo xcode-select -r # s /Applications/Xcode.app
mkdir -p cmake-build-release && cd ./cmake-build-release
sed -i '' '/CMAKE_CXX_COMPILER:FILEPATH/d' ./CMakeCache.txt # remove compiler cache entry just in case travis updated xcode
sed -i '' '/\/\/CXX compiler/d' ./CMakeCache.txt
sed -i '' '/CMAKE_C_COMPILER:FILEPATH/d' ./CMakeCache.txt # remove compiler cache entry just in case travis updated xcode
sed -i '' '/\/\/C compiler/d' ./CMakeCache.txt
cmake -G Ninja -DGEMS_BUILD_MATLAB=OFF ..
cmake -G Ninja -DGEMS_BUILD_MATLAB=OFF .. # again so that AppleClang finds OpenMP
timeout 50m ninja