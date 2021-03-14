#!/usr/bin/env bash

set -e

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name 'CMakeLists.txt' -o -name '*.cmake' \) |
  parallel -j $(nproc) --eta --bar --max-args=1 cmake-format -c $1 -i {}
