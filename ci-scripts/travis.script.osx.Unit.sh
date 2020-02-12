#!/bin/bash

sudo xcode-select -r # s /Applications/Xcode.app
mkdir -p cmake-build-debug && cd ./cmake-build-debug
cmake .. # reconfigure to copy testdata from git-annex
ctest --timeout 300 -j 12
