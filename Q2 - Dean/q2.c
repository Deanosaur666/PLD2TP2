/*

A small college wishes to assign unique identification
numbers to all of its present and future students.
The administration is thinking of using a identifier
but is not sure that there will be enough combinations,
given various constraints that have been placed on what
is an "acceptable" identifier.

Write a parallel MPI program to count the number of 
different six-digit combinations of the
numerals 0-9, given these constraints:

• The first may not be a 0.
• Two consecutive digits may not be the same.
• The sum of the digits may not be 7, 11, or 13

*/

#include <mpi.h>
#include <stdio.h>
#include <stdbool.h>

#define DIGIT_COUNT 6
#define CHAR_TO_NUM(c) c - '0'
#define NUM_TO_CHAR(n)  n + '0'

// these values don't consider the consecutive digit rule
#define MIN_ID 100000
#define MAX_ID 999999

// the sum can't be 7, 11, or 13
int digitsum(char * id) {
    int sum = 0;
    for(int i = 0; i < DIGIT_COUNT; i ++) {
        sum += CHAR_TO_NUM(id[i]);
    }
    return sum;
}

// no consecutive digits can be the same
bool consecutivedigits(char * id) {
    for(int i = 1; i < DIGIT_COUNT; i ++) {
        if(id[i-1] == id[i])
            return true;
    }
    return false;
}

int checkID(int id) {
    char idc[DIGIT_COUNT];
    sprintf(idc, "%d", id);
    if(consecutivedigits(idc))
        return 0;
    int sum = digitsum(idc);
    return (sum != 7 && sum != 11 && sum != 13);

}

int main(int argc, char *argv[]) {
    int count;            /* Solutions found by this proc */
    double elapsed_time;  /* Time to find, count solutions */
    int global_count;     /* Total number of solutions */
    int i;
    int id;               /* Process rank */
    int p;                /* Number of processes */

    MPI_Init (&argc, &argv);

    /* Start timer */
    MPI_Barrier (MPI_COMM_WORLD);
    elapsed_time = - MPI_Wtime();

    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);


    count = 0;
    for (i = MIN_ID + id; i < MAX_ID; i += p)
        count += checkID(i);

    MPI_Reduce (&count, &global_count, 1, MPI_INT, MPI_SUM, 0,
        MPI_COMM_WORLD); 

    /* Stop timer */
    elapsed_time += MPI_Wtime();
    if (!id) {
        printf ("T: %8.6f s\n", elapsed_time);
        fflush (stdout);
    }

    MPI_Finalize();
    return 0;
}