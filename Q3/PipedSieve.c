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

int main (int argc, char *argv[])
{
   int    count;        /* Local prime count */
   double elapsed_time; /* Parallel execution time */
   int    first;        /* Index of first multiple */
   int    global_count; /* Global prime count */
   int    high_value;   /* Highest value on this proc */
   int    i;
   int    id;           /* Process ID number */
   int    index;        /* Index of current prime */
   int    low_value;    /* Lowest value on this proc */
   char  *marked;       /* Portion of 2,...,'n' */
   int    n;            /* Sieving from 2, ..., 'n' */
   int    p;            /* Number of processes */
   int    proc0_size;   /* Size of proc 0's subarray */
   int    prime;        /* Current prime */
   int    nextPrime;    /* Next prime, recieved */
   int    sentPrime;    /* Sent prime */
   int    size;         /* Elements in 'marked' */

   // send and recieve requests and status, for Isend and Irecv
   MPI_Request send_request, recv_request;
   MPI_Status recv_status;

   MPI_Init (&argc, &argv);

   /* Start the timer */

   MPI_Comm_rank (MPI_COMM_WORLD, &id);
   MPI_Comm_size (MPI_COMM_WORLD, &p);
   MPI_Barrier(MPI_COMM_WORLD);
   elapsed_time = -MPI_Wtime();

   if (argc != 2) {
      if (!id) printf ("Command line: %s <m>\n", argv[0]);
      MPI_Finalize();
      exit (1);
   }

   n = atoi(argv[1]);

   /* Figure out this process's share of the array, as
      well as the integers represented by the first and
      last array elements */

   low_value = 2 + id*(n-1)/p;
   high_value = 1 + (id+1)*(n-1)/p;
   size = high_value - low_value + 1;

   /* Bail out if all the primes used for sieving are
      not all held by process 0 */

   proc0_size = (n-1)/p;

   if ((2 + proc0_size) < (int) sqrt((double) n)) {
      if (!id) printf ("Too many processes\n");
      MPI_Finalize();
      exit (1);
   }

   /* Allocate this process's share of the array. */

   marked = (char *) malloc (size);

   if (marked == NULL) {
      printf ("Cannot allocate enough memory\n");
      MPI_Finalize();
      exit (1);
   }

   for (i = 0; i < size; i++)
      marked[i] = 0;
   if (!id)
      index = 0;
   prime = 2;
   do {
      if(id > 0) {
         MPI_Irecv(&nextPrime, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &recv_request);
      }
      if (prime * prime > low_value)
         first = prime * prime - low_value;
      else {
         if (!(low_value % prime))
            first = 0;
         else
            first = prime - (low_value % prime);
      }
      for (i = first; i < size; i += prime)
         marked[i] = 1;
      if (!id) {
         while (marked[++index]);
         prime = index + 2;
      }
      if (p > 1) {
         // use a pipeline instead of a broadcast
         //MPI_Bcast (&prime,  1, MPI_INT, 0, MPI_COMM_WORLD);
         // processes after 1 recieve
         if(id > 0) {
            //MPI_Irecv(&prime, 1, MPI_INT, id - 1, 0, MPI_COMM_WORLD, &recv_request);
            MPI_Wait(&recv_request, &recv_status);
            prime = nextPrime;
         }
         // processes before the last send
         if(id < p-1) {
            sentPrime = prime;
            MPI_Isend(&sentPrime, 1, MPI_INT, id + 1, 0, MPI_COMM_WORLD, &send_request);
         }
      }
   } while (prime * prime <= n);
   count = 0;
   for (i = 0; i < size; i++)
      if (!marked[i])
         count++;

   if (p > 1)
      MPI_Reduce (&count, &global_count, 1, MPI_INT, MPI_SUM,
         0, MPI_COMM_WORLD);
   else
      global_count = count;

   /* Stop the timer */

   elapsed_time += MPI_Wtime();


   /* Print the results */

   if (!id) {
      printf ("C: %d primes under %d\n", global_count, n);
      //printf ("SIEVE (%d) %10.6f\n", p, elapsed_time);
      printf ("T: %10.6f\n", elapsed_time);
   }
   MPI_Finalize ();
   return 0;
}