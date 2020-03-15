#!/usr/bin/env bash

set -e

INSTRUMENTATION="$1"

mkdir -p cmake-build-debug-bionic && cd ./cmake-build-debug-bionic

export PATH="/usr/local/bin:$PATH"

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

  rm -rf ./prebuilt_packages/itk
  wget "https://www.dropbox.com/s/uqysekgefm3vb8x/itk5.0.1_linux_bionic.tar?dl=1" >/dev/null 2>&1
  tar -xf "./itk5.0.1_linux_bionic.tar?dl=1"
  mv ./itk5 ./prebuilt_packages/itk

  # TODO: reenable guis after fixing compile issues
  # TODO: check if the other flags are still needed (originally for the benchmark module)
  cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUIS=OFF -DCMAKE_CROSSCOMPILING=1 -DRUN_HAVE_STD_REGEX=0 -DRUN_HAVE_POSIX_REGEX=0 -DFS_ADD_INSTRUMENTATION=$INSTRUMENTATION ..
fi
#cd ./packages
#wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-3.12.4.tar.gz
#tar -xzf petsc-lite-3.12.4.tar.gz
#cd petsc-lite-3.12.4.tar.gz
#./configure
#make all test
#make install
