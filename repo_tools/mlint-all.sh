#!/usr/bin/env bash

find . \
  \( -name '*.m' \) |
  parallel --max-args=1 /Applications/MATLAB/Matlab.app/bin/maci64/mlint '{}'
