#!/usr/bin/env bash

(
  mkdir -p cmake-build-debug-xenial && cd ./cmake-build-debug-xenial
  sudo apt-get update -y -qq
  sudo apt-get install -y -qq --no-install-suggests --no-install-recommends git wget tcsh build-essential \
    gfortran libblas-dev liblapack-dev zlib1g-dev libxmu-dev \
    libxmu-headers libxi-dev libxt-dev libx11-dev libglu1-mesa-dev \
    mpich python3.5 libboost-all-dev libfltk1.3-dev libeigen3-dev \
    libarmadillo-dev qt5-default libqt5x11extras5-dev curl git-annex lcov libxml2-utils

  #  export LD_LIBRARY_PATH="/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH"

  sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
  sudo apt-get update -y -qq
  sudo apt-get install -y -qq gcc-9 g++-9
  sudo apt-get install -y -qq gfortran-9

  if [[ ! -f "$(pwd)/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz" ]]; then
    echo "Downloading and Installing LLVM"
    wget http://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz >/dev/null 2>&1
    sudo tar -xf clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz --strip-components=1 -C /usr/local
  else
    echo "LLVM archive found; Installing"
    sudo tar -xf clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz --strip-components=1 -C /usr/local
  fi

  export PATH=/usr/local/bin:$PATH

  (
    cd /usr/local
    sudo cat >libs.conf <<"END"
/usr/local/lib
END
  )

  sudo mv libs.conf /etc/ld.so.conf.d/libs.conf
  sudo ldconfig

  #  export PATH="$(pwd)/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-16.04/bin:$PATH"
  #  export LD_LIBRARY_PATH="$(pwd)/clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-16.04/lib:$LD_LIBRARY_PATH"
  export CXX=clang++
  export CC=clang

  sudo update-alternatives --install /usr/local/bin/python python /usr/bin/python3.5 1 --force
  sudo add-apt-repository --yes ppa:fkrull/deadsnakes
  sudo apt-get update
  sudo apt-get install --yes python3.5 python3.5-dev

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
    wget "https://www.dropbox.com/s/drijzemquap0rqk/itk5.0.1_linux_xenial.tar?dl=1" >/dev/null 2>&1
    tar -xf "./itk5.0.1_linux_xenial.tar?dl=1"
    mv ./itk5 ./prebuilt_packages/itk

    sed -i -- 's/#    error Unsupported compiler/\/\//g' ./prebuilt_packages/itk/include/ITK-5.0/itk_compiler_detection.h
  fi
  # TODO: reenable guis after fixing compile issues
  # TODO: check if the other flags are still needed (originally for the benchmark module)
  cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUIS=OFF -DCMAKE_CROSSCOMPILING=1 -DRUN_HAVE_STD_REGEX=0 -DRUN_HAVE_POSIX_REGEX=0 ..
)
