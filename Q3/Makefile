all: sieve

sieve : Sieve.c
	@mpicc Sieve.c -o sieve -Wall -lm

clean:
	@rm -f sieve rr