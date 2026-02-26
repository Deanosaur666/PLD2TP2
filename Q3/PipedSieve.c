/*
 *  Sieve of Eratosthenes
 *  Using pipelines
 */
/*

Implement a new version of the Sieve of Eratosthenes
program in MPI, replacing the broadcast step with a
pipeline of sends and receives. In other words,
instead of process 0 broadcasting the next prime to
all the other processes, it sends the next prime to
process 1, which receives the value, and it sends it
to process 2, and so on. Process 0 does not perform a 
receive, and the process of highest rank does not
perform a send. For each prime found, the maximum
number of communication steps per process is this
way reduced from log p to 2, where p is the number of
processors.

*/

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {

    int    id;           /* Process ID number */
    int    p;            /* Number of processes */
    double elapsed_time; /* Parallel execution time */

    MPI_Init (&argc, &argv);

    /* Start the timer */

    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();

    if (argc != 2) {
        if (!id)
            printf ("Command line: %s <m>\n", argv[0]);
        MPI_Finalize();
        exit (1);
    }


    /* Stop the timer */

    elapsed_time += MPI_Wtime();

    /* Print the results */

   if (!id) {
        //printf ("There are %d primes less than or equal to %d\n", global_count, n);
        //printf ("SIEVE (%d) %10.6f\n", p, elapsed_time);
        printf ("T: %10.6f\n", elapsed_time);
    }

    MPI_Finalize();
    return 0;
}