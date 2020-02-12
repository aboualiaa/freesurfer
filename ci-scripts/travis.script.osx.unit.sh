#!/bin/bash

sudo xcode-select -r # s /Applications/Xcode.app
mkdir -p cmake-build-debug && cd ./cmake-build-debug
ctest --timeout 300 -j 12
