#!/bin/bash

# this script downloads all data from the annex repo
# then copies it on a cache on azure pipelines

sudo apt-get install -y --no-install-suggests --no-install-recommends git-annex
git config --global user.email "ahmed.s.aboualiaa@gmail.com"
git config --global user.name "Ahmed Abou-Aliaa"
git init --bare $CURRENT_WORK_DIR/annex-cache
cd $CURRENT_WORK_DIR/annex-cache
git annex init
git config annex.hardlink true
git annex untrust here
cd $CURRENT_WORK_DIR
git clone --no-recurse-submodules https://www.github.com/aboualiaa/freesurfer
cd freesurfer
git remote add datasrc https://surfer.nmr.mgh.harvard.edu/pub/dist/freesurfer/repo/annex.git
git fetch datasrc
git annex get . || true
git remote add cache $CURRENT_WORK_DIR/annex-cache
git config remote.cache.annex-speculate-present true
git config remote.cache.annex-cost 10
git config remote.cache.annex-pull false
git config remote.cache.annex-push false
git config remote.cache.fetch do-not-fetch-from-this-remote
git annex copy --to cache
