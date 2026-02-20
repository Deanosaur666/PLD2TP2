#! /bin/bash
echo "Q2" > "q2log.txt"
for p in {1..10}
do
	echo "P: $p" >> q2log.txt
	for i in {1..5}
	do
		mpirun -n $p --oversubscribe q2 >> q2log.txt
	done
done
