#! /bin/bash
echo "Q3" > "q3log1.txt"
for p in {1..30}
do
	echo "P: $p" >> q3log1.txt
	echo "P: $p" # so we can monitor progress
	for i in {1..20}
	do
		mpirun -n $p --oversubscribe sieve 65535 >> q3log1.txt
	done
done
