#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    long start;
    long end;
    double partial_sum;
} ThreadData;

void* calculate_pi_part(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    double sum = 0.0;
    double denominator = 1.0 + (2.0 * data->start);

    for (long i = data->start; i < data->end; i++) {
        if (i % 2 == 0)
            sum += 1.0 / denominator;
        else
            sum -= 1.0 / denominator;

        denominator += 2.0;
    }

    data->partial_sum = sum;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <number_of_iterations> <number_of_threads>\n", argv[0]);
        return 1;
    }

    long iterations = atol(argv[1]);
    int num_threads = atoi(argv[2]);

    if (iterations <= 0 || num_threads <= 0) {
        printf("Both arguments must be positive numbers.\n");
        return 1;
    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    long chunk_size = iterations / num_threads;

    clock_t start_time = clock();

    // Create threads
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == num_threads - 1) ? iterations : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, calculate_pi_part, &thread_data[i]);
    } 

    // Wait for threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Combine results
    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pi += thread_data[i].partial_sum;
    }
    pi *= 4.0;

    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Approximation of Pi: %.15f\n", pi);
    printf("Iterations: %ld | Threads: %d | Time: %.6f seconds\n", iterations, num_threads, time_taken);

    return 0;
}