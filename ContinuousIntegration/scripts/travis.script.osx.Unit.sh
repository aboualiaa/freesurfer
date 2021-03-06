#!/usr/bin/env bash

set -e

cd ./cmake-build-debug
ctest --schedule-random --timeout 90 -j $(nproc) -L Unit -T Test --group Continuous
#lcov --capture --directory . --output-file ./coverage.info
#curl -s https://codecov.io/bash -o codecov.sh
#chmod +x ./codecov.sh
#./codecov.sh -f ./coverage.info -t ${CODECOV_TOKEN}
rm -f ./**/testdata.tar.gz
rm -rf ./**/testdata/
ctest -T Submit
