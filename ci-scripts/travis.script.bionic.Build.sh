#!/bin/bash

cd ./cmake-build-debug-bionic
ctest --timeout 300 -j 12
