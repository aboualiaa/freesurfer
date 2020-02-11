#!/bin/bash

ln -s /usr/local/opt/gcc/gcc-9 /usr/local/opt/gcc/bin/gcc
ln -s /usr/local/opt/gcc/g++-9 /usr/local/opt/gcc/bin/g++
./ci-scripts/restore-mtime.sh
./ci-scripts/install-hpx.sh
