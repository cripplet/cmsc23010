#!/bin/bash

CD="/home/mzhang2013/courses/cmsc23010/cmsc23010/hash/code"

cd $CD

# make clean
# make

# ./hash 1 >> $CD/ONE.tsv
# ./hash 2 >> $CD/TWO.tsv
# ./hash 3 0 >> $CD/THREE.tsv
# ./hash 3 1 >> $CD/THREE.tsv
# ./hash 3 2 >> $CD/THREE.tsv
./hash 3 3 .5 >> $CD/THREE.tsv
./hash 3 3 .75 >> $CD/THREE.tsv
./hash 3 3 .9 >> $CD/THREE.tsv
./hash 3 3 .99 >> $CD/THREE.tsv
./hash 3 4 .5 >> $CD/THREE.tsv
./hash 3 4 .75 >> $CD/THREE.tsv
./hash 3 4 .9 >> $CD/THREE.tsv
./hash 3 4 .99 >> $CD/THREE.tsv
./hash 3 5 .5 >> $CD/THREE.tsv
./hash 3 5 .75 >> $CD/THREE.tsv
./hash 3 5 .9 >> $CD/THREE.tsv
./hash 3 5 .99 >> $CD/THREE.tsv
