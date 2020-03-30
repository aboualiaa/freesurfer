#!/usr/bin/env bash

set -e

HOSTING_OS="$1"
HOSTING_COMPILER="$2"

#sudo add-apt-repository --yes ppa:fkrull/deadsnakes    # for python
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test # for gcc 9

sudo apt-get update -y -qq
sudo apt-get install -y -qq --no-install-suggests --no-install-recommends git wget tcsh build-essential \
  gfortran libblas-dev liblapack-dev zlib1g-dev libxmu-dev \
  libxmu-headers libxi-dev libxt-dev libx11-dev libglu1-mesa-dev \
  mpich libboost-all-dev libfltk1.3-dev libeigen3-dev \
  libarmadillo-dev qt5-default libqt5x11extras5-dev curl git-annex lcov libxml2-utils \
  snapd parallel

sudo snapd install shfmt

if [ "$HOSTING_OS" = "xenial" ] || [ "$HOSTING_OS" = "trusty" ]; then
  sudo apt-get install -y -qq python3.5 python3.5-dev
  sudo update-alternatives --install /usr/local/bin/python python /usr/bin/python3.5 1 --force
else
  sudo apt-get install -y -qq python3.6 python3.6-dev
  sudo update-alternatives --install /usr/local/bin/python python /usr/bin/python3.6 1 --force
fi

sudo apt-get install -qq -y gcc-9 g++-9
sudo apt-get install -qq -y gfortran-9

sudo update-alternatives --install /usr/bin/gfortran gfortran /usr/bin/gfortran-9 90

if [ "$HOSTING_COMPILER" = "gcc" ]; then
  sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90 --slave /usr/bin/g++ g++ /usr/bin/g++-9
fi

git config --global user.email "ahmed.s.aboualiaa@gmail.com"
git config --global user.name "Ahmed Abou-Aliaa"
