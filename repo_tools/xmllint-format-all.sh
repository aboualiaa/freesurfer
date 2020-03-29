#!/usr/bin/env bash

find . \
    \( -name '*.xml' \) |
   parallel -j +2 --eta --bar --max-args=1 mllint --noout '{}'

find . \
    \( -name '*.xml' \) |
    parallel -j +2 --eta --bar --max-args=1 mllint --output '{}' --format '{}'
