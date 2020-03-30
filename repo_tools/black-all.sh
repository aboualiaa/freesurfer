#!/usr/bin/env bash

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.py' \) |
  parallel -j +2 --eta --bar --max-args=1 black -q -l 80 {}
