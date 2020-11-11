#!/usr/bin/env bash

set -e

mkdir -p cmake-build-debug-trusty && cd ./cmake-build-debug-trusty

if [[ ! -d "$(pwd)/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-14.04" ]]; then
  wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz >/dev/null 2>&1
  tar -xf clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-14.04.tar.xz
fi
export PATH="$(pwd)/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-14.04/bin:$PATH"
export LD_LIBRARY_PATH="$(pwd)/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-14.04/lib:$LD_LIBRARY_PATH"
export CXX=clang++
export CC=clang

if [[ ! -d "$(pwd)/cmake-3.16.4-Linux-x86_64" ]]; then
  wget https://cmake.org/files/v3.16/cmake-3.16.4-Linux-x86_64.tar.gz >/dev/null 2>&1
  tar -xzf cmake-3.16.4-Linux-x86_64.tar.gz
fi

export PATH="$(pwd)/cmake-3.16.4-Linux-x86_64/bin:$PATH"

wget https://github.com/ninja-build/ninja/releases/download/v1.10.0/ninja-linux.zip >/dev/null 2>&1
sudo unzip ninja-linux.zip -d /usr/local/bin/
sudo update-alternatives --install /usr/bin/ninja ninja /usr/local/bin/ninja 1 --force
rm ninja-linux.zip

if [[ ! -d "$(pwd)/prebuilt_packages" ]]; then
  wget "https://www.dropbox.com/s/5xffk87vm0wb938/linux_packages.tar?dl=1" >/dev/null 2>&1
  tar -xf "./linux_packages.tar?dl=1"

  #  rm -rf ./prebuilt_packages/itk
  #  wget "https://www.dropbox.com/s/drijzemquap0rqk/itk5.0.1_linux_xenial.tar?dl=1" >/dev/null 2>&1
  #  tar -xf "./itk5.0.1_linux_xenial.tar?dl=1"
  #  mv ./itk5 ./prebuilt_packages/itk

  export PATH="/usr/local/bin:$PATH"

  sed -i -- 's/#  error \"Dunno about this gcc\"/\/\//g' ./prebuilt_packages/itk/include/ITK-4.13/vcl_compiler.h
fi
# TODO: reenable guis after fixing compile issues
# TODO: check if the other flags are still needed (originally for the benchmark module)
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DFS_BUILD_GUIS=OFF -DCMAKE_CROSSCOMPILING=1 -DRUN_HAVE_STD_REGEX=0 -DRUN_HAVE_POSIX_REGEX=0 ..
