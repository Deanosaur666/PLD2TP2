#! /bin/bash
echo "Q4" > "q4log.txt"
for p in {1..30}
do
	echo "P: $p" >> q4log.txt
	echo "P: $p" # so we can monitor progress
	for i in {1..20}
	do
		mpirun -n $p --oversubscribe conway 128 500 -D >> q4log.txt
	done
done
