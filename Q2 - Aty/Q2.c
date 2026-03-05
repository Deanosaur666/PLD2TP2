/*Question #2*/

#include "mpi.h"
#include <stdio.h>

int main(int argc, char* argv[]) {
    int count;            /* Solutions found by this proc */
    double elapsed_time;  /* Time to find, count solutions */
    int global_count;     /* Total number of solutions */
    int id;               /* Process rank */
    int p;                /* Number of processes */  

    MPI_Init(&argc, &argv);
/* Start timer */
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);


    count = 0;
    /*MPI cyclic work‑distribution pattern*/
    for (int i = id; i <= 999999; i += p) {       
        if (is_valid(i)) count++;
    } 

    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0,
        MPI_COMM_WORLD);

 /* Stop timer */
    elapsed_time += MPI_Wtime();

    if (id == 0) { 
        printf("Execution time %8.6f\n", elapsed_time); 
        printf("Total valid identifiers: %ld\n", global_count); 
        fflush(stdout); 
    }
    MPI_Finalize();
    return 0;
}

/* validate a 6 digit identifier */
int is_valid(int n) {
    int d[6]; //6 digit identifier

    //breaking a 6 digit int to individual digits
    for (int i = 5; i >= 0; i--) {
        d[i] = n % 10;
        n /= 10;
    }

    //Checking the first‑digit: The first may not be a 0.
    if (d[0] == 0) {
        return 0;
    }
    
    //Two consecutive digits may not be the same
    //Checking duplication: compares each digit with the next one    
    for (int i = 0; i < 5; i++)
        if (d[i] == d[i + 1]) {
            return 0;
        }

    //Checking the sum constraint: The sum of the digits may not be 7, 11, or 13.
    int sum = d[0] + d[1] + d[2] + d[3] + d[4] + d[5];
    if (sum == 7 || sum == 11 || sum == 13) {
        return 0;
    }

    return 1;
}

