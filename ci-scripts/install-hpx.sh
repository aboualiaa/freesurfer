#!/bin/bash

if [ -d ./packages/hpx/cmake-build-release ]; then
  echo "cached hpx found -- nothing to download, installing"
  cd ./packages/hpx/cmake-build-release
  timeout 50m make -j$(nproc) install
  cd ../../../
else
  echo "cached hpx not found -- building from source"
  git clone https://github.com/STEllAR-GROUP/hpx.git packages/hpx
  cd ./packages/hpx
  mkdir cmake-build-release && cd ./cmake-build-release
  cmake .. -DJEMALLOC_ROOT=/usr/local/opt/jemalloc -DHPX_WITH_MALLOC=jemalloc
  timeout 50m make -j$(nproc) install
  cd ../../../
fi