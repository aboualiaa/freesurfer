#!/bin/bash

(
  mkdir -p cmake-build-debug-trusty && cd ./cmake-build-debug-trusty

  sudo apt-get install -y --no-install-suggests --no-install-recommends git wget tcsh build-essential \
    gfortran libblas-dev liblapack-dev zlib1g-dev libxmu-dev \
    libxmu-headers libxi-dev libxt-dev libx11-dev libglu1-mesa-dev \
    mpich2 python3.5 libboost-all-dev libfltk1.3-dev libeigen3-dev \
    libarmadillo-dev qt5-default libqt5x11extras5-dev xz-utils

  curl -SL http://releases.llvm.org/9.0.0/clang%2bllvm-9.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz | tar -xJC .
  mv clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-14.04 clang_9.0.0
  sudo mv clang_9.0.0 /usr/local
  export PATH="/usr/local/clang_9.0.0/bin:$PATH"
  export LD_LIBRARY_PATH="/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH"

  export PATH="/usr/local/bin:$PATH"

  sudo update-alternatives --install /usr/local/bin/python python /usr/bin/python3.5 1 --force
  sudo add-apt-repository --yes ppa:fkrull/deadsnakes
  sudo apt-get update
  sudo apt-get install --yes python3.5 python3.5-dev

  wget https://cmake.org/files/v3.16/cmake-3.16.4-Linux-x86_64.tar.gz
  tar -xzf cmake-3.16.4-Linux-x86_64.tar.gz
  export PATH="$(pwd)/cmake-3.16.4-Linux-x86_64/bin:$PATH"

  wget https://github.com/ninja-build/ninja/releases/download/v1.10.0/ninja-linux.zip
  sudo unzip ninja-linux.zip -d /usr/local/bin/
  sudo update-alternatives --install /usr/bin/ninja ninja /usr/local/bin/ninja 1 --force

  curl --connect-timeout 8 --retry 5 -O http://surfer.nmr.mgh.harvard.edu/pub/data/fspackages/prebuilt/centos7-packages.tar.gz
  tar -xzf centos7-packages.tar.gz
  mv ./packages ./prebuilt_packages

  sed -i -- 's/#  error \"Dunno about this gcc\"/\/\//g' ./prebuilt_packages/itk/4.13.0/include/ITK-4.13/vcl_compiler.h

  # TODO: reenable guis after fixing compile issues
  # TODO: check if the other flags are still needed (originally for the benchmark module)
  cmake -G Ninja -DBUILD_GUIS=OFF -DCMAKE_CROSSCOMPILING=1 -DRUN_HAVE_STD_REGEX=0 -DRUN_HAVE_POSIX_REGEX=0 ..
)
