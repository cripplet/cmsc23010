#!/bin/bash

make clean
make

for ITER in {0..9}
do
	for DIM in 16 32 64 128 256 512 1024
	do
		python gen_arr.py -n $DIM -i $ITER
		for THREAD in 0 1 2 4 8 16 32 64
		do
			./main -n $DIM -c $THREAD -i $ITER
		done
	done
done
