#!/bin/bash

cd ./cmake-build-debug-bionic
ctest --timeout 120 -j 12
