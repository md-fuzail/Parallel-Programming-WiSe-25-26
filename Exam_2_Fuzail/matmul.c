#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

double my_rand(unsigned long* state, double lower, double upper) {
    *state ^= *state >> 12;
    *state ^= *state << 25;
    *state ^= *state >> 27;
    unsigned long x = (*state * 0x2545F4914F6CDD1DULL);
    const double inv = 1.0 / (double)(1ULL << 53);
    double u = (double)(x >> 11) * inv;
    return lower + (upper - lower) * u;
}

unsigned long concatenate(unsigned i, unsigned j) {
    unsigned pow = 10;
    while (j >= pow) pow *= 10;
    return i * pow + j;
}

// Helper to allocate 1D contiguous memory for 2D matrices
double* alloc_matrix(int rows, int cols) {
    double* data = malloc((size_t)rows * cols * sizeof(double));
    if (!data) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    return data;
}

void print_matrix(double* mat, int n, const char* name) {
    printf("%s:\n", name);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%f ", mat[i * n + j]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4) {
        if (rank == 0) fprintf(stderr, "Usage: ./matmul <n> <seed> <verbose>\n");
        MPI_Finalize();
        return 1;
    }

    int n = atoi(argv[1]);
    int seed = atoi(argv[2]);
    int verbose = atoi(argv[3]);

    // Start timed region
    MPI_Barrier(MPI_COMM_WORLD); 
    double start_time = MPI_Wtime();

    // Calculate row distribution for each process
    int *sendcounts = malloc(size * sizeof(int));
    int *displs = malloc(size * sizeof(int));
    int remainder = n % size;
    int sum = 0, my_rows = 0, my_row_start = 0;

    for (int i = 0; i < size; i++) {
        int rows_per_proc = n / size + (i < remainder ? 1 : 0);
        sendcounts[i] = rows_per_proc * n;
        displs[i] = sum;
        
        if (i == rank) {
            my_rows = rows_per_proc;
            my_row_start = sum / n; 
        }
        sum += sendcounts[i];
    }

    // Generate full matrix B locally to avoid network broadcast
    double *mat_b = alloc_matrix(n, n); 
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            unsigned long state = concatenate(i, j) + seed + 1;
            mat_b[i * n + j] = my_rand(&state, 0, 1);
        }
    }

    // Generate only the local horizontal slice of matrix A
    double *local_a = alloc_matrix(my_rows, n);
    double *local_c = alloc_matrix(my_rows, n);
    memset(local_c, 0, (size_t)my_rows * n * sizeof(double));

    for (int i = 0; i < my_rows; i++) {
        int global_row_idx = my_row_start + i;
        for (int j = 0; j < n; j++) {
            unsigned long state = concatenate(global_row_idx, j) + seed + 0;
            local_a[i * n + j] = my_rand(&state, 0, 1);
        }
    }

    // Cache-friendly matrix multiplication (i-k-j loop order)
    for (int i = 0; i < my_rows; i++) {
        for (int k = 0; k < n; k++) {
            double a_val = local_a[i * n + k];
            for (int j = 0; j < n; j++) {
                local_c[i * n + j] += a_val * mat_b[k * n + j];
            }
        }
    }

    // Collect all computed rows back to rank 0
    double *mat_c = NULL;
    if (rank == 0) mat_c = alloc_matrix(n, n);
    
    MPI_Gatherv(local_c, my_rows * n, MPI_DOUBLE, mat_c, sendcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Verify and output results
    if (rank == 0) {
        double checksum = 0.0;
        for (int i = 0; i < n * n; i++) {
            checksum += mat_c[i]; 
        }
        
        checksum = fmod(checksum, 18446744073709551616.0);

        // Print matrices for debugging small dimensions
        if (verbose == 1 && n <= 10) {
            double* full_a = alloc_matrix(n, n);
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    unsigned long state = concatenate(i, j) + seed + 0;
                    full_a[i*n+j] = my_rand(&state, 0, 1);
                }
            }
            print_matrix(full_a, n, "Matrix A");
            print_matrix(mat_b, n, "Matrix B");
            print_matrix(mat_c, n, "Matrix C (Result)");
            free(full_a);
        }
        
        double end_time = MPI_Wtime();

        printf("Checksum: %f\n", checksum);
        printf("Execution time with %d ranks: %.2f s\n", size, end_time - start_time);
        
        free(mat_c);
    }
    
    free(mat_b);
    free(local_a);
    free(local_c);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}