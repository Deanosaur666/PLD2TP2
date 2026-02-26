all: sieve

sieve : Sieve.c
	@mpicc Sieve.c -o sieve -Wall -lm

psieve : PipedSieve.c
	@mpicc PipedSieve.c -o psieve -Wall -lm

clean:
	@rm -f sieve psieve