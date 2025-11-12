#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000000  // 1 million elements

int main(int argc, char** argv) {
    double *a;
    double sum = 0.0;
    double start, end;
    int i;

    // Allocate memory
    a = (double*) malloc(N * sizeof(double));
    if (a == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    // Initialize the vector
    for (i = 0; i < N; i++) {
        a[i] = 1.0;  // simple values for easy verification
    }

    // Start timing
    start = omp_get_wtime();

    // Parallel summation without reduction(+:sum)
    #pragma omp parallel for schedule(static)
    for (i = 0; i < N; i++) {
        sum += a[i];
    }

    // End timing
    end = omp_get_wtime();

    printf("Result (sum): %f\n", sum);
    printf("Elapsed time: %f seconds\n", end - start);

    free(a);
    return 0;
}
