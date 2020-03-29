#!/usr/bin/env bash

find . \
  \( -name '*.py' \) |
  parallel -j +2 --eta --bar --max-args=1 black -q -l 80 {}
