#!/bin/bash

cmake . -DFS_PACKAGES_DIR=packages -DBUILD_GUIS=OFF && make -j4
