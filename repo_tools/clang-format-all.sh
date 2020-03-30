#!/usr/bin/env bash

find . \
  -not \( -path ./packages -prune \) \
  -not \( -path "./cmake-build-*" -prune \) \
  \( -name '*.c' \
  -o -name '*.C' \
  -o -name '*.H' \
  -o -name '*.c++' \
  -o -name '*.h++' \
  -o -name '*.cxx' \
  -o -name '*.hxx' \
  -o -name '*.cc' \
  -o -name '*.cpp' \
  -o -name '*.h' \
  -o -name '*.hh' \
  -o -name '*.txx' \
  -o -name '*.tpp' \
  -o -name '*.hpp' \) |
  parallel -j +2 --eta --bar --max-args=1 /usr/local/bin/clang-format -i '{}'
