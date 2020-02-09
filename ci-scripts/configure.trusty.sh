#!/bin/bash

sudo apt-get install -y git wget tcsh build-essential \
  gfortran libblas-dev liblapack-dev zlib1g-dev libxmu-dev \
  libxmu-headers libxi-dev libxt-dev libx11-dev libglu1-mesa-dev \
  mpich2 python3.5 libboost-all-dev libfltk1.3-dev libeigen3-dev \
  libarmadillo-dev qt5-default libqt5x11extras5-dev

sudo mv /usr/bin/python /usr/bin/python.bak
sudo mv /usr/bin/python3.5 /usr/bin/python

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-9 g++-9

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 --slave /usr/bin/g++ g++ /usr/bin/g++-9

wget https://cmake.org/files/v3.13/cmake-3.13.5-Linux-x86_64.tar.gz
tar -xzvf cmake-3.13.5-Linux-x86_64.tar.gz
export PATH="$(pwd)/cmake-3.13.5-Linux-x86_64/bin:$PATH"

curl --connect-timeout 8 --retry 5 -O http://surfer.nmr.mgh.harvard.edu/pub/data/fspackages/prebuilt/centos7-packages.tar.gz
tar -xzf centos7-packages.tar.gz
mv ./packages ./prebuilt_packages

#cd ./packages
#wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-3.12.4.tar.gz
#tar -xzf petsc-lite-3.12.4.tar.gz
#cd petsc-lite-3.12.4.tar.gz
#./configure
#make all test
#make install
