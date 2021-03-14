#!/bin/bash

/Applications/Polyspace/MATLAB.app/bin/maci64/mlint $1 >>mlint.tmp.txt 2>&1
/Users/aboualiaa/Desktop/.temp/repos/freesurfer/repo_tools/mtc mlint.tmp.txt $1
rm mlint.tmp.txt
