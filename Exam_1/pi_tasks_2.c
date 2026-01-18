#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int num_tasks;
int task_counter = 0;
double sum_pi = 0.0;
long long *thread_counts;

double compute_pi(long precision) {
    double pi = 0.0;
    double step = 1.0 / precision;
    
    for (long i = 0; i < precision; i++) {
        double x = (i + 0.5) * step;
        pi += 4.0 / (1.0 + x * x);
    }
    
    return pi * step;
}

unsigned long xorshift64(unsigned long *state) {
    unsigned long x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

void create_tasks(unsigned long *state, long lower, long upper) {
    int tid = omp_get_thread_num();
    
    // Compute pi with random precision
    long precision = lower + (xorshift64(state) % (upper - lower + 1));
    double pi_val = compute_pi(precision);
    
    #pragma omp atomic
    thread_counts[tid]++;
    
    #pragma omp atomic
    sum_pi += pi_val;
    
    // Create 1-4 new tasks
    int new_tasks = 1 + (xorshift64(state) % 4);
    
    for (int i = 0; i < new_tasks; i++) {
        int should_create = 0;
        #pragma omp critical
        {
            if (task_counter < num_tasks) {
                task_counter++;
                should_create = 1;
            }
        }
        
        if (should_create) {
            #pragma omp task
            {
                unsigned long local_state = *state;
                create_tasks(&local_state, lower, upper);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s num_tasks num_threads lower upper seed\n", argv[0]);
        return 1;
    }
    
    num_tasks = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    long lower = atol(argv[3]);
    long upper = atol(argv[4]);
    unsigned long seed = atol(argv[5]);
    
    omp_set_num_threads(num_threads);
    
    thread_counts = (long long *)calloc(num_threads, sizeof(long long));
    
    double start = omp_get_wtime();
    
    #pragma omp parallel shared(seed, lower, upper)
    {
        #pragma omp single
        {
            task_counter = 1;
            unsigned long state = seed;
            #pragma omp task
            {
                create_tasks(&state, lower, upper);
            }
        }
    }
    
    double end = omp_get_wtime();
    
    double avg_pi = sum_pi / num_tasks;
    
    printf("Average pi: %.10f\n", avg_pi);
    for (int i = 0; i < num_threads; i++) {
        printf("Thread %d computed %lld tasks\n", i, thread_counts[i]);
    }
    printf("Execution took %.4f s\n", end - start);
    
    free(thread_counts);
    
    return 0;
}
