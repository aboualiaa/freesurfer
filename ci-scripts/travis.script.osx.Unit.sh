#!/usr/bin/env bash

cd ./cmake-build-debug
timeout 50m ninja
ctest --schedule-random --timeout 90 -j $(nproc) -LE Expensive -T Test --group Continuous
lcov --capture --directory . --output-file ./coverage.info
curl -s https://codecov.io/bash -o codecov.sh
chmod +x ./codecov.sh
./codecov.sh -f ./coverage.info -t ${CODECOV_TOKEN}
rm -f ./**/testdata.tar.gz
rm -rf ./**/testdata/
ctest -T Submit