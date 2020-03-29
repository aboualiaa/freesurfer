#!/usr/bin/env bash

find . \
  \( -name '*.py' \) |
  parallel --max-args=1 black -l 80 {}
