#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000000  // size of the array

int main() {
    int *a;
    int target = 42; // the value to search for
    a = (int*) malloc(N * sizeof(int));
    if (a == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    // Initialize array with random integers 0-99
    srand(0);
    for (int i = 0; i < N; i++) {
        a[i] = rand() % 100;
    }

    double start = omp_get_wtime();

    int found_index = -1; // to store the first found index

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        // Check if cancellation has been requested
        #pragma omp cancellation point for

        if (a[i] == target) {
            int id = omp_get_thread_num();

            // Store index safely using critical section
            #pragma omp critical
            {
                if (found_index == -1) {
                    found_index = i;
                    printf("Thread %d found target at index %d\n", id, i);
                }
            }

            // Cancel the remaining iterations
            #pragma omp cancel for
        }
    }
    // #pragma omp barrier

    double end = omp_get_wtime();

    if (found_index == -1)
        printf("Target %d not found in the array.\n", target);

    printf("Elapsed time: %f seconds\n", end - start);

    free(a);
    return 0;
}