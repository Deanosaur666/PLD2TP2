
#include <mpi.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank (MPI_COMM_WORLD, &rank); 
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    int send_data, recv_data;
    MPI_Request send_request, recv_request;
    MPI_Status send_status, recv_status;

    if (size < 2) {
        if (rank == 0) {
            printf("This program requires at least two processes.\n");
            MPI_Finalize();
            return 0;
        }
    }
    
    int partner = (rank + 1) % size; // Simple ring communication
    send_data = rank; // Send own rank as data

    // Non-blocking send and receive
    MPI_Isend(&send_data, 1, MPI_INT, partner, 0, MPI_COMM_WORLD, &send_request);
    MPI_Irecv(&recv_data, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &recv_request);

    // Wait for both operations to complete MPI_Wait(&send_request, &send_status);
    MPI_Wait(&recv_request, &recv_status);

    printf("Process %d sent %d and received %d\n", rank, send_data, recv_data);

    MPI_Finalize();
    return 0;
}