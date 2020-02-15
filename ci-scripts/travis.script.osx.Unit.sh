#!/bin/bash

cd ./cmake-build-debug;
timeout 50m ninja
ctest --schedule-random --timeout 90 -j $(nproc) -LE Expensive -T Test;
lcov --capture --directory . --output-file ./coverage.info;
bash <(curl -s https://codecov.io/bash) -f ./coverage.info -t ${CODECOV_TOKEN};
rm -f ./**/testdata.tar.gz;
rm -rf ./**/testdata/;
ctest -T Submit;
