#!/bin/bash

cd ./cmake-build-debug;
ctest --schedule-random --timeout 120 -j 6;
lcov --capture --directory . --output-file ./coverage.info;
bash <(curl -s https://codecov.io/bash) -f ./coverage.info -t ${CODECOV_TOKEN};
rm -f ./**/testdata.tar.gz;
rm -rf ./**/testdata/;
