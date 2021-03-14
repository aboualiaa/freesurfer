#!/usr/bin/env bash

set -e

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.xml' \) |
  parallel -j $(nproc) --eta --bar --max-args=1 xmllint --noout '{}'

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.xml' \) |
  parallel -j $(nproc) --eta --bar --max-args=1 xmllint --output '{}' --format '{}'
