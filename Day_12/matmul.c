#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- HELPER FUNCTIONS ---
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

void fillArray(double* data, int n, int value, int seed) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            unsigned long state = concatenate(i, j) + seed + value;
            data[i * n + j] = my_rand(&state, 0, 1);
        }
    }
}

// Helper to allocate a contiguous matrix
double* alloc_matrix(int rows, int cols) {
    double* data = (double*)malloc((size_t)rows * cols * sizeof(double));
    if (data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for size %dx%d\n", rows, cols);
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

// --- MAIN PROGRAM ---

int main(int argc, char* argv[]) {
    // 1. Initialize MPI
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4) {
        if (rank == 0) {
            fprintf(stderr, "Usage: mpirun -np <p> ./matmul <n> <seed> <verbose>\n");
        }
        MPI_Finalize();
        return 1;
    }

    int n = atoi(argv[1]);
    int seed = atoi(argv[2]);
    int verbose = atoi(argv[3]);

    // 3. Start Timer (Must include initialization!)
    MPI_Barrier(MPI_COMM_WORLD); 
    double start_time = MPI_Wtime();

    // 4. Determine Load Balancing (Scatterv Setup)
    int* sendcounts = NULL;
    int* displs = NULL;
    
    if (rank == 0) {
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        
        int remainder = n % size;
        int sum = 0;
        for (int i = 0; i < size; i++) {
            int rows = n / size;
            if (i < remainder) {
                rows++;
            }
            sendcounts[i] = rows * n; // Total doubles to send (rows * cols)
            displs[i] = sum;
            sum += sendcounts[i];
        }
    }

    // Determine my own local height (how many rows do I get?)
    int my_rows = n / size;
    if (rank < (n % size)) {
        my_rows++;
    }

    // 5. Allocation & Initialization
    double* mat_a = NULL;
    double* mat_b = alloc_matrix(n, n); // Everyone needs full B
    double* mat_c = NULL;               // Only Rank 0 needs full C (initially)
    
    // Buffers for local computation
    double* local_a = alloc_matrix(my_rows, n); 
    double* local_c = alloc_matrix(my_rows, n);
    // Initialize local_c to 0.0 to be safe (though logic below overwrites it usually)
    memset(local_c, 0, (size_t)my_rows * n * sizeof(double));

    if (rank == 0) {
        mat_a = alloc_matrix(n, n);
        mat_c = alloc_matrix(n, n);

        fillArray(mat_a, n, 0, seed);
        fillArray(mat_b, n, 1, seed);
    }

    // 6. Communication Phase

    // A: Broadcast Full Matrix B to everyone
    MPI_Bcast(mat_b, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // B: Scatter Matrix A (using Scatterv for irregular row counts)
    MPI_Scatterv(mat_a, sendcounts, displs, MPI_DOUBLE, 
                 local_a, my_rows * n, MPI_DOUBLE, 
                 0, MPI_COMM_WORLD);

    // 7. Computation Phase (Parallel Matrix Multiplication)
    // C_local = A_local * B
    // Optimized loop order: i-k-j
    for (int i = 0; i < my_rows; i++) {
        for (int k = 0; k < n; k++) {
            double a_val = local_a[i * n + k];
            for (int j = 0; j < n; j++) {
                local_c[i * n + j] += a_val * mat_b[k * n + j];
            }
        }
    }

    // 8. Gather Results
    MPI_Gatherv(local_c, my_rows * n, MPI_DOUBLE,
                mat_c, sendcounts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
                
    // 9. Output & Verification (Rank 0 Only)
    if (rank == 0) {
        // CHANGED: Use double, do not cast to unsigned long
        double checksum = 0.0;
        for (int i = 0; i < n * n; i++) {
            checksum += mat_c[i]; 
        }

        if (verbose == 1 && n <= 10) {
            print_matrix(mat_a, n, "Matrix A");
            print_matrix(mat_b, n, "Matrix B");
            print_matrix(mat_c, n, "Matrix C (Result)");
        }
        
        double end_time = MPI_Wtime();
        double elapsed = end_time - start_time;

        // CHANGED: Print as float (%f)
        printf("Checksum: %f\n", checksum);
        printf("Execution time with %d ranks: %.2f s\n", size, elapsed);
        
        free(mat_a);
        free(mat_c);
        free(sendcounts);
        free(displs);
    }
    
    // Cleanup
    free(mat_b);
    free(local_a);
    free(local_c);

    MPI_Finalize();
    return 0;
}