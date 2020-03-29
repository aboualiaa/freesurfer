#!/usr/bin/env bash

find . \
    \( -name '*.xml' \) |
    parallel --max-args=1 xmllint --noout '{}'

find . \
    \( -name '*.xml' \) |
    parallel --max-args=1 xmllint --output '{}' --format '{}'
