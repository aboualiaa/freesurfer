#!/bin/bash

ln -s /usr/local/opt/gcc/gcc-9 /usr/local/opt/gcc/bin/gcc
ln -s /usr/local/opt/gcc/g++-9 /usr/local/opt/gcc/bin/g++
sudo sed -i -- 's/#  error \"Dunno about this gcc\"/\/\//g' /usr/local/include/ITK-5.0/itk_compiler_detection.h
./ci-scripts/restore-mtime.sh
./ci-scripts/install-hpx.sh
