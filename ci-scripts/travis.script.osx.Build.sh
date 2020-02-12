#!/bin/bash

sudo xcode-select -r # s /Applications/Xcode.app
mkdir -p cmake-build-debug && cd ./cmake-build-debug
sed -i '' '/CMAKE_CXX_COMPILER:FILEPATH/d' ./CMakeCache.txt # remove compiler cache entry just in case travis updated xcode
sed -i '' '/\/\/CXX compiler/d' ./CMakeCache.txt
sed -i '' '/CMAKE_C_COMPILER:FILEPATH/d' ./CMakeCache.txt # remove compiler cache entry just in case travis updated xcode
sed -i '' '/\/\/C compiler/d' ./CMakeCache.txt
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGEMS_BUILD_MATLAB=OFF ..
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DGEMS_BUILD_MATLAB=OFF .. # again so that AppleClang finds OpenMP
timeout 50m ninja -k 0
cd ..
git remote add datasrc https://surfer.nmr.mgh.harvard.edu/pub/dist/freesurfer/repo/annex.git
git annex init
git fetch datasrc
git annex get . || true
cd ./cmake-build-debug
cmake ..
ctest --timeout 300 -j $(nproc)
lcov --capture --directory . --output-file ./coverage.info
bash <(curl -s https://codecov.io/bash) -f ./coverage.info -t ${CODECOV_TOKEN}
rm -f ./**/testdata.tar.gz
