#!/bin/bash

sudo xcode-select -r # s /Applications/Xcode.app
git remote add datasrc https://surfer.nmr.mgh.harvard.edu/pub/dist/freesurfer/repo/annex.git
git fetch datasrc
git annex get . || true
cd ./cmake-build-debug
cmake ..
