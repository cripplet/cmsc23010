#!/bin/bash

CD="/home/mzhang/Desktop/2013/winter/cmsc230/git/cmsc23010/locks/code"

cd $CD

# make clean
# make

# ./lock 2 2> $CD/IDLE.tsv
# ./lock 4 2> $CD/SCAL.tsv
# ./lock 8 2> $CD/FAIR.tsv
# ./lock 16 2> $CD/POHD.tsv
./lock 32 2> $CD/PSCL.tsv
