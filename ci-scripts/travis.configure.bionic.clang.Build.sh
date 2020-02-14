#!/bin/bash
set -e
(
  mkdir -p cmake-build-debug-bionic && cd ./cmake-build-debug-bionic

  sudo apt-get install -y --no-install-suggests --no-install-recommends git wget tcsh build-essential \
    gfortran libblas-dev liblapack-dev zlib1g-dev libxmu-dev \
    libxmu-headers libxi-dev libxt-dev libx11-dev libglu1-mesa-dev \
    mpich python3.6 libboost-all-dev libfltk1.3-dev libeigen3-dev \
    libarmadillo-dev qt5-default libqt5x11extras5-dev curl

  wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz >/dev/null 2>&1
  tar -xf clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
  export PATH="$(pwd)/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04/bin:$PATH"
  export LD_LIBRARY_PATH="$(pwd)/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04/lib:$LD_LIBRARY_PATH"
  export CXX=clang++
  export CC=clang

  #  sudo add-apt-repository --yes ppa:fkrull/deadsnakes
  sudo apt-get update
  sudo apt-get install --yes python3.6 python3.6-dev
  sudo update-alternatives --install /usr/local/bin/python python /usr/bin/python3.6 1 --force

  wget https://cmake.org/files/v3.16/cmake-3.16.4-Linux-x86_64.tar.gz >/dev/null 2>&1
  tar -xzf cmake-3.16.4-Linux-x86_64.tar.gz
  export PATH="$(pwd)/cmake-3.16.4-Linux-x86_64/bin:$PATH"

  wget https://github.com/ninja-build/ninja/releases/download/v1.10.0/ninja-linux.zip >/dev/null 2>&1
  sudo unzip ninja-linux.zip -d /usr/local/bin/
  sudo update-alternatives --install /usr/bin/ninja ninja /usr/local/bin/ninja 1 --force

  wget "https://www.dropbox.com/s/5xffk87vm0wb938/linux_packages.tar?dl=1" >/dev/null 2>&1
  tar -xf "./linux_packages.tar?dl=1"

  rm -rf ./prebuilt_packages/itk
  wget "https://www.dropbox.com/s/drijzemquap0rqk/itk5.0.1_linux_bionic.tar?dl=1" >/dev/null 2>&1
  tar -xf "./itk5.0.1_linux_bionic.tar?dl=1"
  mv ./itk5 ./prebuilt_packages/itk

  sed -i -- 's/#    error Unsupported compiler/\/\//g' ./prebuilt_packages/itk/include/ITK-5.0/itk_compiler_detection.h

  # TODO: reenable guis after fixing compile issues
  # TODO: check if the other flags are still needed (originally for the benchmark module)
  cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUIS=OFF -DCMAKE_CROSSCOMPILING=1 -DRUN_HAVE_STD_REGEX=0 -DRUN_HAVE_POSIX_REGEX=0 ..

)
