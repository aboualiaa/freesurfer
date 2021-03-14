#!/usr/bin/env bash

set -e

cd ./cmake-build-debug-trusty
ninja
rm -rf ./clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-*
rm -rf ./cmake-3.16.4-Linux-x86*
rm -rf "./linux_packages.tar?dl=1"
rm -rf ./ninja-linux.zip
rm -rf ./prebuilt_packages
