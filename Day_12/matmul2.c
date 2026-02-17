#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- HELPER FUNCTIONS --- */
double my_rand(unsigned long* state, double lower, double upper) {
    unsigned long x;
    const double inv = 1.0 / (double)(1ULL << 53);
    double u;
    
    *state ^= *state >> 12;
    *state ^= *state << 25;
    *state ^= *state >> 27;
    x = (*state * 0x2545F4914F6CDD1DULL);
    u = (double)(x >> 11) * inv;
    return lower + (upper - lower) * u;
}
unsigned long concatenate(unsigned i, unsigned j) {
    unsigned pow = 10;
    while (j >= pow) pow *= 10;
    return i * pow + j;
}

/* Helper to allocate a contiguous matrix */
double* alloc_matrix(int rows, int cols) {
    double* data = (double*)malloc((size_t)rows * cols * sizeof(double));
    if (data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for size %dx%d\n", rows, cols);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    return data;
}

void print_matrix(double* mat, int n, const char* name) {
    int i, j;
    printf("%s:\n", name);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%f ", mat[i * n + j]);
        }
        printf("\n");
    }
}

/* --- MAIN PROGRAM --- */

int main(int argc, char* argv[]) {
    int rank, size;
    int n, seed, verbose;
    double start_time, end_time, elapsed;
    int *sendcounts = NULL;
    int *displs = NULL;
    int remainder, sum, rows_per_proc;
    int i, j, k; 
    int my_rows;
    double *mat_b = NULL;
    double *mat_c = NULL;
    double *local_a = NULL;
    double *local_c = NULL;
    double a_val;
    unsigned long state; 
    /* Start Global Offset for parallel generation */
    int my_row_start; 

    /* 1. Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* 2. Parse Arguments */
    if (argc < 4) {
        if (rank == 0) fprintf(stderr, "Usage: ./matmul <n> <seed> <verbose>\n");
        MPI_Finalize();
        return 1;
    }

    n = atoi(argv[1]);
    seed = atoi(argv[2]);
    verbose = atoi(argv[3]);

    MPI_Barrier(MPI_COMM_WORLD); 
    start_time = MPI_Wtime();

    /* 4. Determine Load Balancing */
    /* We need these arrays for Gather later */
    sendcounts = (int*)malloc(size * sizeof(int));
    displs = (int*)malloc(size * sizeof(int));
    
    remainder = n % size;
    sum = 0;
    for (i = 0; i < size; i++) {
        rows_per_proc = n / size;
        if (i < remainder) rows_per_proc++;
        
        sendcounts[i] = rows_per_proc * n;
        displs[i] = sum;
        
        /* Save my specific start row index for parallel generation */
        if (i == rank) {
            my_rows = rows_per_proc;
            my_row_start = sum / n; /* Convert count back to row index */
        }
        
        sum += sendcounts[i];
    }

    /* 5. Parallel Allocation & Initialization */
    /* STRATEGY CHANGE: Everyone generates B locally. 
       Redundant computation is faster than broadcasting 512MB over network. */
    mat_b = alloc_matrix(n, n); 
    
    /* Generate B (Everyone does this in parallel) */
    /* Note: Since B is read-only and same for everyone, this is valid */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            state = concatenate(i, j) + seed + 1; /* value=1 for B */
            mat_b[i * n + j] = my_rand(&state, 0, 1);
        }
    }

    /* Generate ONLY my part of A (local_a) */
    local_a = alloc_matrix(my_rows, n);
    local_c = alloc_matrix(my_rows, n);
    memset(local_c, 0, (size_t)my_rows * n * sizeof(double));

    for (i = 0; i < my_rows; i++) {
        int global_row_idx = my_row_start + i;
        for (j = 0; j < n; j++) {
            state = concatenate(global_row_idx, j) + seed + 0; /* value=0 for A */
            local_a[i * n + j] = my_rand(&state, 0, 1);
        }
    }

    /* 6. Communication Phase */
    /* Removed Bcast(B) and Scatter(A) because we just generated them! */
    /* This saves massive bandwidth and latency. */

    /* 7. Computation Phase (i-k-j loop) */
    for (i = 0; i < my_rows; i++) {
        for (k = 0; k < n; k++) {
            a_val = local_a[i * n + k];
            for (j = 0; j < n; j++) {
                local_c[i * n + j] += a_val * mat_b[k * n + j];
            }
        }
    }

    /* 8. Gather Results (The only unavoidable communication) */
    if (rank == 0) mat_c = alloc_matrix(n, n);
    
    MPI_Gatherv(local_c, my_rows * n, MPI_DOUBLE,
                mat_c, sendcounts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    /* 9. Output & Verification */
    if (rank == 0) {
        double checksum = 0.0;
        for (i = 0; i < n * n; i++) {
            checksum += mat_c[i]; 
        }

        /* Verbose output logic (only for small N) */
        if (verbose == 1 && n <= 10) {
            double* full_a = alloc_matrix(n, n);
            /* Re-generate A just for printing (since we only have pieces) */
             for (i = 0; i < n; i++) {
                for (j = 0; j < n; j++) {
                    state = concatenate(i, j) + seed + 0;
                    full_a[i*n+j] = my_rand(&state, 0, 1);
                }
            }
            print_matrix(full_a, n, "Matrix A");
            print_matrix(mat_b, n, "Matrix B");
            print_matrix(mat_c, n, "Matrix C (Result)");
            free(full_a);
        }
        
        end_time = MPI_Wtime();
        elapsed = end_time - start_time;

        printf("Checksum: %f\n", checksum);
        printf("Execution time with %d ranks: %.2f s\n", size, elapsed);
        
        free(mat_c);
    }
    
    /* Cleanup */
    free(mat_b);
    free(local_a);
    free(local_c);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}