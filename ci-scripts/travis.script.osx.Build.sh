#!/usr/bin/env bash

set -e

mkdir -p cmake-build-debug && cd ./cmake-build-debug
timeout 50m ninja
