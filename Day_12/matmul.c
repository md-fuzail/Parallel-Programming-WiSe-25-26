#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

// Global parameters
int N;
int SEED;
int VERBOSE;


double my_rand(unsigned long* state, double lower, double upper)
{
    *state ^= *state >> 12;
    *state ^= *state << 25;
    *state ^= *state >> 27;
    unsigned long x = (*state * 0x2545F4914F6CDD1DULL);
    const double inv = 1.0 / (double)(1ULL << 53);
    double u = (double)(x >> 11) * inv;
    return lower + (upper - lower) * u;
}

unsigned long concatenate(unsigned x, unsigned y)
{
    unsigned long pow = 10;
    while (y >= pow)
        pow *= 10;
    return x * pow + y;
}

void printMatrix(double** arr, const char* name)
{
    printf("%s:\n", name);
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            printf("%f ", arr[i][j]);
        }
        printf("\n");
    }
}

void fillArray(double** arr, int offset)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            unsigned long state = concatenate(i, j) + SEED + offset;
            arr[i][j] = my_rand(&state, 0, 1);
        }
    }
}

void multiply_matrices(double* row_a, double* result_row, double** matrix_b)
{
    for (int i = 0; i < N; i++)
    {
        result_row[i] = 0;
        for (int j = 0; j < N; j++)
        {
            result_row[i] += row_a[j] * matrix_b[j][i];
        }
    }
}

int main(int argc, char* argv[])
{
    int rank, size;
    int ierr;
    double start_time, end_time;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4) {
        if (rank == 0) {
            printf("Usage: mpirun -np <p> %s <n> <seed> <verbose>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    N = atoi(argv[1]);
    SEED = atoi(argv[2]);
    VERBOSE = atoi(argv[3]);

    start_time = MPI_Wtime();

    double** matrix_a = NULL;
    double** matrix_b = NULL;
    double** matrix_c = NULL;
    double* buffer = NULL;
    double* answer = NULL;

    // Allocation common to all
    matrix_b = (double**)malloc(N * sizeof(double*));
    for (int i = 0; i < N; i++) {
        matrix_b[i] = (double*)malloc(N * sizeof(double));
    }
    answer = (double*)malloc(N * sizeof(double));

    MPI_Status status;

    if (rank == 0) // MASTER
    {
        matrix_a = (double**)malloc(N * sizeof(double*));
        matrix_c = (double**)malloc(N * sizeof(double*));
        for (int i = 0; i < N; i++) {
            matrix_a[i] = (double*)malloc(N * sizeof(double));
            matrix_c[i] = (double*)malloc(N * sizeof(double));
        }

        fillArray(matrix_a, 0);
        fillArray(matrix_b, 1);

        if (VERBOSE == 1 && N <= 10) {
            printMatrix(matrix_a, "Matrix A");
            printMatrix(matrix_b, "Matrix B");
        }

        // Broadcast Matrix B
        for (int i = 0; i < N; i++) {
            ierr = MPI_Bcast(matrix_b[i], N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }

        int row_a_counter = 0;
        
        // Initial dispatch
        for (int i = 1; i < size; i++)
        {
            if (row_a_counter < N) {
                // Send with tag = row_index + 1
                ierr = MPI_Send(matrix_a[row_a_counter], N, MPI_DOUBLE, i, row_a_counter + 1, MPI_COMM_WORLD);
                row_a_counter++;
            } else {
                // Send STOP signal (tag 0)
                ierr = MPI_Send(matrix_b[0], N, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            }
        }

        // Receive Loop
        for (int i = 0; i < N; i++)
        {
            ierr = MPI_Recv(answer, N, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int finished_row = status.MPI_TAG;
            int sender = status.MPI_SOURCE;
            
            memcpy(matrix_c[finished_row], answer, N * sizeof(double));

            if (row_a_counter < N) {
                ierr = MPI_Send(matrix_a[row_a_counter], N, MPI_DOUBLE, sender, row_a_counter + 1, MPI_COMM_WORLD);
                row_a_counter++;
            } else {
                ierr = MPI_Send(matrix_b[0], N, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
            }
        }

        if (VERBOSE == 1 && N <= 10) {
            printMatrix(matrix_c, "Matrix C (Result)");
        }

        double checksum = 0;
        for(int i=0; i<N; i++) {
            for(int j=0; j<N; j++) {
                checksum += matrix_c[i][j];
            }
        }
        printf("Checksum: %f\n", checksum);
        
        end_time = MPI_Wtime();
        printf("Execution time with %d ranks: %.2f s\n", size, end_time - start_time);

        // Cleanup Master
        for (int i=0; i<N; i++) { free(matrix_a[i]); free(matrix_c[i]); }
        free(matrix_a); free(matrix_c);
    }
    else // WORKER
    {
        buffer = (double*)malloc(N * sizeof(double));

        // Receive Matrix B
        for (int i = 0; i < N; i++) {
            ierr = MPI_Bcast(matrix_b[i], N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        }

        while (1)
        {
            ierr = MPI_Recv(buffer, N, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int tag = status.MPI_TAG;

            if (tag == 0) break; // STOP

            int row = tag - 1;
            multiply_matrices(buffer, answer, matrix_b);
            
            // Send back using actual row index as tag
            ierr = MPI_Send(answer, N, MPI_DOUBLE, 0, row, MPI_COMM_WORLD);
        }
        free(buffer);
    }

    for (int i=0; i<N; i++) free(matrix_b[i]);
    free(matrix_b);
    free(answer);

    MPI_Finalize();
    return 0;
}