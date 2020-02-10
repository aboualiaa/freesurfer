#!/bin/bash

(
  mkdir -p cmake-build-debug-bionic && cd ./cmake-build-debug-bionic

  sudo apt-get install -y --no-install-suggests --no-install-recommends git wget tcsh build-essential \
    gfortran libblas-dev liblapack-dev zlib1g-dev libxmu-dev \
    libxmu-headers libxi-dev libxt-dev libx11-dev libglu1-mesa-dev \
    mpich2 python3.5 libboost-all-dev libfltk1.3-dev libeigen3-dev \
    libarmadillo-dev qt5-default libqt5x11extras5-dev

  export PATH="/usr/local/bin:$PATH"

  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  sudo apt-get update
  sudo apt-get install -y gcc-9 g++-9

  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 --slave /usr/bin/g++ g++ /usr/bin/g++-9

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

  #cd ./packages
  #wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-3.12.4.tar.gz
  #tar -xzf petsc-lite-3.12.4.tar.gz
  #cd petsc-lite-3.12.4.tar.gz
  #./configure
  #make all test
  #make install
)
