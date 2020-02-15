#!/bin/bash

./ci-scripts/restore-mtime.sh
./ci-scripts/install-hpx.sh

sudo rm -f /usr/local/include/ITK-5.0/itk_compiler_detection.h
sudo rm -f /usr/local/include/ITK-5.0/compilers/*
sudo cp ./ci-scripts/itk_compiler_detection/itk_compiler_detection.h /usr/local/include/ITK-5.0/
sudo cp ./ci-scripts/itk_compiler_detection/compilers/* /usr/local/include/ITK-5.0/compilers/

sudo xcode-select -r # s /Applications/Xcode.app
export PATH="$(pwd)/hpx-install/:$PATH"
mkdir -p cmake-build-debug && cd ./cmake-build-debug
sed -i '' '/CMAKE_CXX_COMPILER:FILEPATH/d' ./CMakeCache.txt || true # remove compiler cache entry just in case travis updated xcode
sed -i '' '/\/\/CXX compiler/d' ./CMakeCache.txt || true
sed -i '' '/CMAKE_C_COMPILER:FILEPATH/d' ./CMakeCache.txt || true # remove compiler cache entry just in case travis updated xcode
sed -i '' '/\/\/C compiler/d' ./CMakeCache.txt || true
cmake -G Ninja -DBUILD_GUIS=OFF -DCMAKE_BUILD_TYPE=Debug -DGEMS_BUILD_MATLAB=OFF ..
cmake -G Ninja -DBUILD_GUIS=OFF -DCMAKE_BUILD_TYPE=Debug -DGEMS_BUILD_MATLAB=OFF .. # again so that AppleClang finds OpenMP
