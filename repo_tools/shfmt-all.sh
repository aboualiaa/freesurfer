#!/usr/bin/env bash

set -e

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.sh' \
  -o -name '*.bash' \) |
  parallel -j +2 --eta --bar --max-args=1 shfmt -i 2 -ci -w {}
