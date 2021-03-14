#!/usr/bin/env bash

set -e

#./scripts/restore-mtime.sh
./ci-scripts/install-hpx.sh

sudo rm -f /usr/local/include/ITK-5.0/itk_compiler_detection.h
sudo rm -f /usr/local/include/ITK-5.0/compilers/*
sudo cp ./ci-scripts/itk_compiler_detection/itk_compiler_detection.h /usr/local/include/ITK-5.0/
sudo cp ./ci-scripts/itk_compiler_detection/compilers/* /usr/local/include/ITK-5.0/compilers/

sudo xcode-select -r # s /Applications/Xcode.app
export PATH="$(pwd)/hpx-install/:$PATH"
mkdir -p cmake-build-debug && cd ./cmake-build-debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DFS_INTEGRATION_TESTING=ON -DFS_GEMS_BUILD_MATLAB=OFF ..
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DFS_INTEGRATION_TESTING=ON -DFS_GEMS_BUILD_MATLAB=OFF .. # again so that AppleClang finds OpenMP
