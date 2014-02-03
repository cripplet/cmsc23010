#!/bin/bash

make clean
make

# experiment one
for ITER in {0..4}
do
	for W in 25 50 100 200 400 800
	do
		for N in 1 8 16
		do
			T=`echo "(2 ^ 24) / ($N * $W)" | bc -l`
			echo "./firewall $T $N $W 1 $ITER 1 >> exp_01.out"
			./firewall $T $N $W 1 $ITER 1 >> output_data/exp_01.out
			./firewall $T $N $W 1 $ITER 3 >> output_data/exp_01.out
		done
	done
done

# experiment two
for ITER in {0..4}
do
	for N in 1 2 4 8 16 32
	do
		T=`echo "(2 ^ 20) / ($N)" | bc -l`
		echo "./firewall $T $N 1 1 $ITER 2 >> exp_02.out"
		./firewall $T $N 1 1 $ITER 2 >> output_data/exp_02.out
	done
done

#experiment four
for ITER in {0..4}
do
	T=`echo "2 ^ 17" | bc -l`
	for W in 1000 2000 4000 8000
	do
		for N in 1 2 4 8 16 32 64
		do
			echo "./firewall $T $N $W 1 $ITER 2 >> exp_04.out"
			./firewall $T $N $W 1 $ITER 2 >> output_data/exp_04.out
		done
	done
done

#experiment five
for ITER in {0..4}
do
	T=`echo "2 ^ 17" | bc -l`
	for W in 1000 2000 4000 8000
	do
		for N in 1 2 4 8 16 32 64
		do
			echo "./firewall $T $N $W 0 $ITER 2 >> exp_05.out"
			./firewall $T $N $W 0 $ITER 2 >> output_data/exp_05.out
		done
	done
done
