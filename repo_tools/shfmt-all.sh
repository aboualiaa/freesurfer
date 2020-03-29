#!/usr/bin/env bash

find . \
  \( -name '*.sh' \
  -o -name '*.tcsh' \
  -o -name '*.csh' \) |
  parallel --max-args=1 shfmt -i 4 -w {}
