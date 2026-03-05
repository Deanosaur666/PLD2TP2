#! /bin/bash
# Pipelined Sieve
echo "--- Pipelined sieve ---"
echo "Q3" > "q3log2.txt"
for p in {1..30}
do
	echo "P: $p" >> q3log2.txt
	echo "P: $p" # so we can monitor progress
	for i in {1..20}
	do
		mpirun -n $p --oversubscribe psieve 65535 >> q3log2.txt
	done
done