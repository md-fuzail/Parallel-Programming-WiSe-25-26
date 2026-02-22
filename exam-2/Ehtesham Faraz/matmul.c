#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

double my_rand(unsigned long* state, double lower, double upper) {
    *state ^= *state >> 12;
    *state ^= *state << 25;
    *state ^= *state >> 27;
    unsigned long x = (*state * 0x2545F4914F6CDD1DULL);
    const double inv = 1.0 / (double)(1ULL << 53);
    double u = (double)(x >> 11) * inv;
    return lower + (upper - lower) * u;
}

unsigned concatenate(unsigned x, unsigned y) {
    unsigned pow = 10;
    while (y >= pow) pow *= 10;
    return x * pow + y;
}

int main(int argc, char* argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4) {
        if (rank == 0) printf("Usage: %s <n> <seed> <verbose>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int n = atoi(argv[1]);
    int seed = atoi(argv[2]);
    int verbose = atoi(argv[3]);

    int base_rows = n / size;
    int remainder = n % size;
    int my_rows = base_rows + (rank < remainder ? 1 : 0);

    int my_offset;
    if (rank < remainder) {
        my_offset = rank * (base_rows + 1);
    } else {
        my_offset = remainder * (base_rows + 1) + (rank - remainder) * base_rows;
    }

    double *local_A = (double*)malloc(my_rows * n * sizeof(double));
    double *local_C = (double*)calloc(my_rows * n, sizeof(double));
    
    double *local_B_slice = (double*)malloc(my_rows * n * sizeof(double));
    double *B = (double*)malloc(n * n * sizeof(double));

    int *recvcounts = (int*)malloc(size * sizeof(int));
    int *displs = (int*)malloc(size * sizeof(int));
    
    int current_disp = 0;
    for (int i = 0; i < size; i++) {
        int rows_i = base_rows + (i < remainder ? 1 : 0);
        recvcounts[i] = rows_i * n; 
        displs[i] = current_disp;
        current_disp += recvcounts[i];
    }

    double *C = NULL;
    if (rank == 0) {
        C = (double*)malloc(n * n * sizeof(double));
    }

    double start_time = MPI_Wtime();

    for (int i = 0; i < my_rows; i++) {
        int global_i = my_offset + i;
        for (int j = 0; j < n; j++) {
            unsigned long state = concatenate(global_i, j) + seed + 0;
            local_A[i * n + j] = my_rand(&state, 0, 1);
        }
    }

    for (int i = 0; i < my_rows; i++) {
        int global_i = my_offset + i;
        for (int j = 0; j < n; j++) {
            unsigned long state = concatenate(global_i, j) + seed + 1;
            local_B_slice[i * n + j] = my_rand(&state, 0, 1);
        }
    }

    MPI_Allgatherv(local_B_slice, my_rows * n, MPI_DOUBLE, 
                   B, recvcounts, displs, MPI_DOUBLE, 
                   MPI_COMM_WORLD);

    free(local_B_slice);

    const double* restrict_B = B;

    for (int i = 0; i < my_rows; i++) {
        for (int k = 0; k < n; k++) {
            double temp = local_A[i * n + k];
            for (int j = 0; j < n; j++) {
                local_C[i * n + j] += temp * restrict_B[k * n + j];
            }
        }
    }

    MPI_Gatherv(local_C, my_rows * n, MPI_DOUBLE, 
                C, recvcounts, displs, MPI_DOUBLE, 
                0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        if (verbose == 1 && n <= 10) {
            printf("Matrix A:\n");
            for(int i=0; i<n; i++) {
                for(int j=0; j<n; j++) printf("%f ", local_A[i*n + j]);
                printf("\n");
            }
            printf("\n");
            printf("Matrix B:\n");
            for(int i=0; i<n; i++) {
                for(int j=0; j<n; j++) printf("%f ", B[i*n + j]);
                printf("\n");
            }
            printf("\n");
            printf("Matrix C (Result):\n");
            for(int i=0; i<n; i++) {
                for(int j=0; j<n; j++) printf("%f ", C[i*n + j]);
                printf("\n");
            }
            printf("\n");
        }

        double checksum = 0;
        for (long i = 0; i < (long)n * n; i++) checksum += C[i];

        printf("Checksum: %f\n", checksum);
        printf("Execution time with %d ranks: %.4f s\n", size, end_time - start_time);

        free(C);
    }

    free(local_A);
    free(local_C);
    free(B);
    free(recvcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}