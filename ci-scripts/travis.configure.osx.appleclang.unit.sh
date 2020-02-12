#!/bin/bash

cd ./cmake-build-debug
git remote add datasrc https://surfer.nmr.mgh.harvard.edu/pub/dist/freesurfer/repo/annex.git
git annex get . || true
