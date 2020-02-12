#!/bin/bash

cd ./cmake-build-debug-trusty
ctest --timeout 300 -j 12
