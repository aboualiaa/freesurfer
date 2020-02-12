#!/bin/bash

cd ./cmake-build-debug-xenial
ctest --timeout 300 -j 12
