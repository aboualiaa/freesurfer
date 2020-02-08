#!/bin/bash

alias python='python3'
export PATH="/usr/local/opt/python/libexec/bin:$PATH"
./ci-scripts/install-ninja.sh
./ci-scripts/restore-mtime.sh
./ci-scripts/install-hpx.sh
