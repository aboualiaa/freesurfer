#!/bin/bash

/Applications/MATLAB/Matlab.app/bin/maci64/mlint $1 >> mlint.tmp.txt 2>&1
/Users/aboualiaa/Library/Developer/Xcode/DerivedData/mtc-aqilmjelbvkirgcmgagolelbijms/Build/Products/Debug/mtc mlint.tmp.txt $1
rm mlint.tmp.txt
