all: conway

conway : conway.c
	@mpicc conway.c -o conway -Wall -lm -lncurses

clean:
	@rm -f conway