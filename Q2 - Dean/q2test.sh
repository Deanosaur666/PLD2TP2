#! /bin/bash
echo "Q2" > "q2log.txt"
for p in {1..30}
do
	echo "P: $p" >> q2log.txt
	echo "P: $p" # so we can monitor progress
	for i in {1..20}
	do
		mpirun -n $p --oversubscribe q2 >> q2log.txt
	done
done
