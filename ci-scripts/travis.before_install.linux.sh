#!/bin/bash

curl --connect-timeout 8 --retry 5 -O http://surfer.nmr.mgh.harvard.edu/pub/data/fspackages/prebuilt/centos7-packages.tar.gz
tar -xzf centos7-packages.tar.gz
