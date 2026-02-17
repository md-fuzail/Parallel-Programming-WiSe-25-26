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

    /* --- HARDCODED PARAMETERS FOR EDUMPI --- */
    /* CHANGE THESE VALUES HERE MANUALLY */
    n = 8000;      /* Matrix Size (Set to 4 for testing, 8000 for exam) */
    seed = 42;     /* Seed (Always 42 for exam) */
    verbose = 0;   /* 0 = Summary only, 1 = Print Matrices (Only if n<=10) */
    MPI_Barrier(MPI_COMM_WORLD); 
    start_time = MPI_Wtime();

    /* 3. Determine Load Balancing */
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

    /* 4. Parallel Allocation & Initialization */
    /* Everyone generates B locally (Redundant Comp > Network Transfer) */
    mat_b = alloc_matrix(n, n); 
    
    /* Generate B (Everyone does this in parallel) */
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

    /* 5. Computation Phase (i-k-j loop with Tiling Optimization) */
    /* Simple i-k-j is fast enough for <15s, but Tiling ensures consistency */
    {
        int tileSize = 64; 
        int ii, kk, jj;
        int i_limit, k_limit, j_limit;

        for (i = 0; i < my_rows; i += tileSize) {
            for (k = 0; k < n; k += tileSize) {
                for (j = 0; j < n; j += tileSize) {
                    
                    i_limit = (i + tileSize < my_rows) ? (i + tileSize) : my_rows;
                    k_limit = (k + tileSize < n) ? (k + tileSize) : n;
                    j_limit = (j + tileSize < n) ? (j + tileSize) : n;

                    for (ii = i; ii < i_limit; ii++) {
                        for (kk = k; kk < k_limit; kk++) {
                            double a_val = local_a[ii * n + kk];
                            for (jj = j; jj < j_limit; jj++) {
                                local_c[ii * n + jj] += a_val * mat_b[kk * n + jj];
                            }
                        }
                    }
                }
            }
        }
    }

    /* 6. Gather Results (The only unavoidable communication) */
    if (rank == 0) mat_c = alloc_matrix(n, n);
    
    MPI_Gatherv(local_c, my_rows * n, MPI_DOUBLE,
                mat_c, sendcounts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    /* 7. Output & Verification */
    if (rank == 0) {
        double checksum = 0.0;
        for (i = 0; i < n * n; i++) {
            checksum += mat_c[i]; 
        }

        /* Verbose output logic (only for small N) */
        if (verbose == 1 && n <= 10) {
            double* full_a = alloc_matrix(n, n);
            /* Re-generate A just for printing */
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