#!/bin/bash

(
  mkdir -p cmake-build-debug-bionic && cd ./cmake-build-debug-bionic

  sudo apt-get install -y --no-install-suggests --no-install-recommends git wget tcsh build-essential \
    gfortran libblas-dev liblapack-dev zlib1g-dev libxmu-dev \
    libxmu-headers libxi-dev libxt-dev libx11-dev libglu1-mesa-dev \
    mpich python3.6 libboost-all-dev libfltk1.3-dev libeigen3-dev \
    libarmadillo-dev qt5-default libqt5x11extras5-dev curl

  #  curl -SL http://releases.llvm.org/9.0.0/clang%2bllvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz | tar -xJC .
  #  mv clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-18.04 clang_9.0.0
  #  sudo mv clang_9.0.0 /usr/local
  #  export PATH="/usr/local/clang_9.0.0/bin:$PATH"
  #  export LD_LIBRARY_PATH="/usr/local/clang_9.0.0/lib:$LD_LIBRARY_PATH"

  # To Install llvm/clang 9.0
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
  sudo apt-add-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-9.0 main"
  sudo apt update
  sudo apt install -y clang-9.0 llvm-9.0 lld-9.0 lldb-9.0
  sudo apt install clang-{format,tidy,tools}-9.0 clang-9.0-doc clang-9.0-examples

  # Remove all existing alternatives
  sudo update-alternatives --remove-all llvm
  sudo update-alternatives --remove-all clang
  sudo update-alternatives --remove-all gcc

  # llvm-9.0 and clang-9.0
  sudo update-alternatives \
    --install /usr/lib/llvm llvm /usr/lib/llvm-9.0 20 \
    --slave /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-9.0 \
    --slave /usr/bin/llvm-ar llvm-ar /usr/bin/llvm-ar-9.0 \
    --slave /usr/bin/llvm-as llvm-as /usr/bin/llvm-as-9.0 \
    --slave /usr/bin/llvm-bcanalyzer llvm-bcanalyzer /usr/bin/llvm-bcanalyzer-9.0 \
    --slave /usr/bin/llvm-cov llvm-cov /usr/bin/llvm-cov-9.0 \
    --slave /usr/bin/llvm-diff llvm-diff /usr/bin/llvm-diff-9.0 \
    --slave /usr/bin/llvm-dis llvm-dis /usr/bin/llvm-dis-9.0 \
    --slave /usr/bin/llvm-dwarfdump llvm-dwarfdump /usr/bin/llvm-dwarfdump-9.0 \
    --slave /usr/bin/llvm-extract llvm-extract /usr/bin/llvm-extract-9.0 \
    --slave /usr/bin/llvm-link llvm-link /usr/bin/llvm-link-9.0 \
    --slave /usr/bin/llvm-mc llvm-mc /usr/bin/llvm-mc-9.0 \
    --slave /usr/bin/llvm-mcmarkup llvm-mcmarkup /usr/bin/llvm-mcmarkup-9.0 \
    --slave /usr/bin/llvm-nm llvm-nm /usr/bin/llvm-nm-9.0 \
    --slave /usr/bin/llvm-objdump llvm-objdump /usr/bin/llvm-objdump-9.0 \
    --slave /usr/bin/llvm-ranlib llvm-ranlib /usr/bin/llvm-ranlib-9.0 \
    --slave /usr/bin/llvm-readobj llvm-readobj /usr/bin/llvm-readobj-9.0 \
    --slave /usr/bin/llvm-rtdyld llvm-rtdyld /usr/bin/llvm-rtdyld-9.0 \
    --slave /usr/bin/llvm-size llvm-size /usr/bin/llvm-size-9.0 \
    --slave /usr/bin/llvm-stress llvm-stress /usr/bin/llvm-stress-9.0 \
    --slave /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-9.0 \
    --slave /usr/bin/llvm-tblgen llvm-tblgen /usr/bin/llvm-tblgen-9.0

  sudo update-alternatives \
    --install /usr/bin/clang clang /usr/bin/clang-9.0 20 \
    --slave /usr/bin/clang++ clang++ /usr/bin/clang++-9.0 \
    --slave /usr/bin/lld lld /usr/bin/lld-9.0 \
    --slave /usr/bin/clang-format clang-format /usr/bin/clang-format-9.0 \
    --slave /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-9.0 \
    --slave /usr/bin/clang-tidy-diff.py clang-tidy-diff.py /usr/bin/clang-tidy-diff-9.0.py \
    --slave /usr/bin/clang-include-fixer clang-include-fixer /usr/bin/clang-include-fixer-9.0 \
    --slave /usr/bin/clang-offload-bundler clang-offload-bundler /usr/bin/clang-offload-bundler-9.0 \
    --slave /usr/bin/clangd clangd /usr/bin/clangd-9.0 \
    --slave /usr/bin/clang-check clang-check /usr/bin/clang-check-9.0 \
    --slave /usr/bin/scan-view scan-view /usr/bin/scan-view-9.0 \
    --slave /usr/bin/clang-apply-replacements clang-apply-replacements /usr/bin/clang-apply-replacements-9.0 \
    --slave /usr/bin/clang-query clang-query /usr/bin/clang-query-9.0 \
    --slave /usr/bin/modularize modularize /usr/bin/modularize-9.0 \
    --slave /usr/bin/sancov sancov /usr/bin/sancov-9.0 \
    --slave /usr/bin/c-index-test c-index-test /usr/bin/c-index-test-9.0 \
    --slave /usr/bin/clang-reorder-fields clang-reorder-fields /usr/bin/clang-reorder-fields-9.0 \
    --slave /usr/bin/clang-change-namespace clang-change-namespace /usr/bin/clang-change-namespace-9.0 \
    --slave /usr/bin/clang-import-test clang-import-test /usr/bin/clang-import-test-9.0 \
    --slave /usr/bin/scan-build scan-build /usr/bin/scan-build-9.0 \
    --slave /usr/bin/scan-build-py scan-build-py /usr/bin/scan-build-py-9.0 \
    --slave /usr/bin/clang-cl clang-cl /usr/bin/clang-cl-9.0 \
    --slave /usr/bin/clang-rename clang-rename /usr/bin/clang-rename-9.0 \
    --slave /usr/bin/find-all-symbols find-all-symbols /usr/bin/find-all-symbols-9.0 \
    --slave /usr/bin/lldb lldb /usr/bin/lldb-9.0 \
    --slave /usr/bin/lldb-server lldb-server /usr/bin/lldb-server-9.0

  export PATH="/usr/local/bin:$PATH"

  sudo update-alternatives --install /usr/local/bin/python python /usr/bin/python3.6 1 --force
  sudo add-apt-repository --yes ppa:fkrull/deadsnakes
  sudo apt-get update
  sudo apt-get install --yes python3.6 python3.6-dev

  wget https://cmake.org/files/v3.16/cmake-3.16.4-Linux-x86_64.tar.gz > /dev/null  2>&1
  tar -xzf cmake-3.16.4-Linux-x86_64.tar.gz
  export PATH="$(pwd)/cmake-3.16.4-Linux-x86_64/bin:$PATH"

  wget https://github.com/ninja-build/ninja/releases/download/v1.10.0/ninja-linux.zip > /dev/null  2>&1
  sudo unzip ninja-linux.zip -d /usr/local/bin/
  sudo update-alternatives --install /usr/bin/ninja ninja /usr/local/bin/ninja 1 --force

  wget "https://www.dropbox.com/s/5xffk87vm0wb938/linux_packages.tar?dl=1" > /dev/null 2>&1
  tar -xf "./linux_packages.tar?dl=1"

  sed -i -- 's/#  error \"Dunno about this gcc\"/\/\//g' ./prebuilt_packages/itk/4.13.0/include/ITK-4.13/vcl_compiler.h

  # TODO: reenable guis after fixing compile issues
  # TODO: check if the other flags are still needed (originally for the benchmark module)
  cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_GUIS=OFF -DCMAKE_CROSSCOMPILING=1 -DRUN_HAVE_STD_REGEX=0 -DRUN_HAVE_POSIX_REGEX=0 ..

)
