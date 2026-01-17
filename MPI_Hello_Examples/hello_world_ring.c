#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int rank, size;
    int message;

    // Initialize MPI
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Init failed\n");
        return EXIT_FAILURE;
    }

    // Get rank and size
    if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Comm_rank failed\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (MPI_Comm_size(MPI_COMM_WORLD, &size) != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Comm_size failed\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Implement ring communication: pass a message around the ring
    if (size < 2) {
        fprintf(stderr, "Error: Need at least 2 processes for ring communication\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (rank == 0) {
        // Start the ring with initial value
        message = 42;
        printf("Process %d/%d: Starting ring with message = %d\n", rank, size, message);

        // Send to next process
        if (MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
            fprintf(stderr, "Error: MPI_Send failed from process %d\n", rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Receive from last process (completing the ring)
        if (MPI_Recv(&message, 1, MPI_INT, size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {
            fprintf(stderr, "Error: MPI_Recv failed for process %d\n", rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        printf("Process %d/%d: Received final message = %d\n", rank, size, message);
    } else {
        // Receive from previous process
        if (MPI_Recv(&message, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE) != MPI_SUCCESS) {
            fprintf(stderr, "Error: MPI_Recv failed for process %d\n", rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Increment the message
        message++;

        printf("Process %d/%d: Received message = %d, incrementing to %d\n",
               rank, size, message - 1, message);

        // Send to next process (or back to 0 if this is the last process)
        int next_rank = (rank + 1) % size;
        if (MPI_Send(&message, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
            fprintf(stderr, "Error: MPI_Send failed from process %d\n", rank);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    // Finalize MPI
    if (MPI_Finalize() != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Finalize failed\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
