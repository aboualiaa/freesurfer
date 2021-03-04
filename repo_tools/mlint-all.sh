#!/usr/bin/env bash

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.m' \) |
  parallel -j $(nproc) --eta --bar --max-args=1 /Applications/Polyspace/MATLAB.app/bin/maci64/mlint '{}'
