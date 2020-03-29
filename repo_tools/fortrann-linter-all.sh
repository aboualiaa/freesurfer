#!/usr/bin/env bash

find . \
  \( -name '*.f' -o -name '*.f90' \) |
  parallel -j +2 --eta --bar --max-args=1 fortran-linter -i {}
