#!/usr/bin/env bash

set -e

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.py' \) |
  parallel -j $(nproc) --eta --bar --max-args=1 black -q -l 80 {}
