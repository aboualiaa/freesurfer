#!/usr/bin/env bash

#./ci-scripts/restore-mtime.sh
./ci-scripts/install-hpx.sh

sudo xcode-select -r # s /Applications/Xcode.app
export PATH="$(pwd)/hpx-install/:$PATH"
mkdir -p cmake-build-debug && cd ./cmake-build-debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DGEMS_BUILD_MATLAB=OFF ..
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DGEMS_BUILD_MATLAB=OFF .. # again so that AppleClang finds OpenMP
