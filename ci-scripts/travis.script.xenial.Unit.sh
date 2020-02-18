#!/bin/bash

cd ./cmake-build-debug-xenial
timeout 50m ninja
ctest --schedule-random --timeout 90 -j $(nproc) -LE Expensive -T Test --group Continuous
#lcov --capture --directory . --output-file ./coverage.info;
#curl -s https://codecov.io/bash -o codecov.sh
#chmod +x ./codecov.sh
#./codecov.sh -f ./coverage.info -t ${CODECOV_TOKEN};
rm -f ./**/testdata.tar.gz
rm -rf ./**/testdata/
rm -rf ./clang+llvm-9.0.0-x86_64-linux-gnu-ubuntu-*
rm -rf ./cmake-3.16.4-Linux-x86*
rm -rf "./linux_packages.tar?dl=1"
rm -rf ./ninja-linux.zip
rm -rf ./prebuilt_packages
ctest -T Submit
