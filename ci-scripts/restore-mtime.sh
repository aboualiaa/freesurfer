#!/bin/bash
git clone https://github.com/MestreLion/git-tools.git
export PATH="$(pwd)/git-tools:$PATH"
git-restore-mtime
LAST_PATH=$(pwd)
for d in ./packages/* ; do (cd "$d" && git-restore-mtime); done
cd $LAST_PATH
