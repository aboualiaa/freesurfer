#!/usr/bin/env bash

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.m' \) |
  parallel -j +2 --eta --bar --max-args=1 Applications/MATLAB/Matlab.app/bin/maci64/mlint '{}'
