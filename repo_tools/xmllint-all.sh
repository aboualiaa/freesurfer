#!/usr/bin/env bash

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.xml' \) |
  parallel -j +2 --eta --bar --max-args=1 xmllint --noout '{}'

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.xml' \) |
  parallel -j +2 --eta --bar --max-args=1 xmllint --output '{}' --format '{}'
