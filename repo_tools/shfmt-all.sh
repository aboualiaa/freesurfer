#!/usr/bin/env bash

find . \
  \( -name '*.sh' \
  -o -name '*.tcsh' \
  -o -name '*.csh' \) |
  parallel -j +2 --eta --bar --max-args=1 hfmt -i 4 -w {}
