#!/usr/bin/env bash

./repo_tools/clang-format-all.sh # c, c++
./repo_tools/black-all.sh        # python
./repo_tools/xmllint-all.sh      # xml
./repo_tools/shfmt-all.sh        # shell
#./repo_tools/mlint-all.sh        # matlab
