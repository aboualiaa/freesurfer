#!/bin/bash

#cmake . -DFS_PACKAGES_DIR=packages -DBUILD_GUIS=OFF && make -j$(nproc)
cmake -DCMAKE_CROSSCOMPILING=1 -DRUN_HAVE_STD_REGEX=0 -DRUN_HAVE_POSIX_REGEX=0 ..
make -j$(nproc)
