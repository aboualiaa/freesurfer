#!/usr/bin/env bash

#set -e

./repo_tools/clang-format-all.sh # c, c++
./repo_tools/black-all.sh        # python
./repo_tools/xmllint-all.sh      # xml
./repo_tools/shfmt-all.sh        # shell
./repo_tools/cmake-format-all.sh ./repo_tools/cmake-format.json
#./repo_tools/mlint-all.sh        # matlab
