all: conway conway-debug

conway : conway.c
	@mpicc conway.c -o conway -Wall -lm -lncurses

conway-debug : conway.c
	@mpicc -g conway.c -o conway-debug -Wall -lm -lncurses

clean:
	@rm -f conway conway-debug