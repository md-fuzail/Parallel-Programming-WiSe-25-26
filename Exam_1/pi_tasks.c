#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Shared state */
static int total_tasks_created = 0;
static double pi_sum = 0.0;

/* Compute pi using the integral method */
double compute_pi(long num_steps)
{
    double step = 1.0 / (double) num_steps;
    double sum = 0.0;

    for (long i = 0; i < num_steps; i++) {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    return step * sum;
}

/* Task function */
void pi_task(int num_tasks, long lower, long upper, unsigned int seed,
             int *tasks_per_thread)
{
    int tid = omp_get_thread_num();
    tasks_per_thread[tid]++;

    long steps = lower + (rand_r(&seed) % (upper - lower + 1));
    int spawn_count = (rand_r(&seed) % 4) + 1;

    for (int i = 0; i < spawn_count; i++) {
        
        int my_task_id;
        #pragma omp atomic capture
        {
            my_task_id = total_tasks_created;
            total_tasks_created++;
        }

        if (my_task_id < num_tasks) {
            unsigned int child_seed = rand_r(&seed);
            #pragma omp task firstprivate(child_seed)
            pi_task(num_tasks, lower, upper, child_seed, tasks_per_thread);
        } else {
            break;
        }
    }
    double pi = compute_pi(steps);

    #pragma omp atomic
    pi_sum += pi;
}

int main(int argc, char *argv[])
{
    if (argc != 6) {
        printf("Usage: %s num_tasks num_threads lower upper seed\n", argv[0]);
        return 1;
    }

    int num_tasks   = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    long lower      = atol(argv[3]);
    long upper      = atol(argv[4]);
    unsigned int seed = (unsigned int) atoi(argv[5]);

    omp_set_num_threads(num_threads);
    int *tasks_per_thread = calloc(num_threads, sizeof(int));

    double start = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp atomic write
            total_tasks_created = 1;

            #pragma omp task firstprivate(seed)
            pi_task(num_tasks, lower, upper, seed, tasks_per_thread);
        }
    }

    double end = omp_get_wtime();

    long total_executed = 0;
    for(int i=0; i<num_threads; i++) 
    total_executed += tasks_per_thread[i];

    double avg_pi = pi_sum / total_executed;

    printf("Average pi: %.10f\n", avg_pi);
    for (int i = 0; i < num_threads; i++) {
        printf("Thread %d computed %d tasks\n", i, tasks_per_thread[i]);
    }
    printf("Execution took %.4f s\n", end - start);

    free(tasks_per_thread);
    return 0;
}