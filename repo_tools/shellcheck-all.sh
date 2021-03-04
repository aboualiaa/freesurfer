#!/usr/bin/env bash

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.sh' \
  -o -name '*.tcsh' \
  -o -name '*.csh' \) |
  parallel -j $(nproc) --eta --bar --max-args=1 shellcheck -o all {}
