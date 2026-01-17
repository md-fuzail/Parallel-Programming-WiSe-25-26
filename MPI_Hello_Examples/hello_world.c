#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int world_rank, world_size;

    // Initialize MPI
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Init failed\n");
        return EXIT_FAILURE;
    }

    // Get rank and size
    if (MPI_Comm_rank(MPI_COMM_WORLD, &world_rank) != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Comm_rank failed\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    if (MPI_Comm_size(MPI_COMM_WORLD, &world_size) != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Comm_size failed\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    printf("Hello world from processor %s, rank %d"
       " out of %d processors\n",
       processor_name, world_rank, world_size);


    // Finalize MPI
    if (MPI_Finalize() != MPI_SUCCESS) {
        fprintf(stderr, "Error: MPI_Finalize failed\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
