#!/usr/bin/env bash

set -e

mkdir -p cmake-build-debug-bionic && cd ./cmake-build-debug-bionic

#git config --global user.email "ahmed.s.aboualiaa@gmail.com"
#git config --global user.name "Ahmed Abou-Aliaa"
#
#git remote add datasrc https://surfer.nmr.mgh.harvard.edu/pub/dist/freesurfer/repo/annex.git
#git fetch datasrc
#git annex get . || true

export PATH="/usr/local/bin:$PATH"

#wget https://cmake.org/files/v3.16/cmake-3.16.4-Linux-x86_64.tar.gz >/dev/null 2>&1
#tar -xzf cmake-3.16.4-Linux-x86_64.tar.gz
export PATH="$(pwd)/cmake-3.16.4-Linux-x86_64/bin:$PATH"

wget https://github.com/ninja-build/ninja/releases/download/v1.10.0/ninja-linux.zip >/dev/null 2>&1
sudo unzip ninja-linux.zip -d /usr/local/bin/
sudo update-alternatives --install /usr/bin/ninja ninja /usr/local/bin/ninja 1 --force

#wget "https://www.dropbox.com/s/5xffk87vm0wb938/linux_packages.tar?dl=1" >/dev/null 2>&1
#tar -xf "./linux_packages.tar?dl=1"
#
#rm -rf ./prebuilt_packages/itk
#wget "https://www.dropbox.com/s/uqysekgefm3vb8x/itk5.0.1_linux_bionic.tar?dl=1" >/dev/null 2>&1
#tar -xf "./itk5.0.1_linux_bionic.tar?dl=1"
#mv ./itk5 ./prebuilt_packages/itk

# TODO: reenable guis after fixing compile issues
# TODO: check if the other flags are still needed (originally for the benchmark module)
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUIS=OFF -DCMAKE_CROSSCOMPILING=1 -DRUN_HAVE_STD_REGEX=0 -DRUN_HAVE_POSIX_REGEX=0 ..
