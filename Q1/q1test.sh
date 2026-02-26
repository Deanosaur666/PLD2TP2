#! /bin/bash
echo "Q1" > "q1log.txt"
for p in {1..30}
do
	echo "P: $p" >> q1log.txt
	echo "P: $p" # so we can monitor progress
	for i in {1..20}
	do
		mpirun -n $p --oversubscribe sati >> q1log.txt
	done
done
