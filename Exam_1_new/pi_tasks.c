#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Completely created together */

/* Global variables */
static int total_tasks_created = 0;
static double pi_sum = 0.0;
static int *tasks_per_thread = NULL;
static int g_num_tasks;
static long g_lower;
static long g_upper;

/* Provided random number function */
unsigned long my_rand(unsigned long* state, unsigned long lower, unsigned long upper) {
    *state ^= *state >> 12;
    *state ^= *state << 25;
    *state ^= *state >> 27;
    unsigned long result = (*state * 0x2545F4914F6CDD1DULL);
    unsigned long range = (upper > lower) ? (upper - lower) : 0UL;
    return (range > 0) ? (result % range + lower) : lower;
}

/* Calculate pi */
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

/* Recursive task function */
void pi_task(unsigned long seed)
{
    int tid = omp_get_thread_num();

    /* increment task counter for this thread */
    #pragma omp atomic
    tasks_per_thread[tid]++;

    /* Generate random load (inclusive g_lower to g_upper) and number of children (1-4) */
    long steps = my_rand(&seed, g_lower, g_upper + 1);
    int spawn_count = my_rand(&seed, 1, 5);

    double pi = compute_pi(steps);

    /* add result to global sum */
    #pragma omp atomic
    pi_sum += pi;

    /* spawn child tasks */
    for (int i = 0; i < spawn_count; i++) {
        
        int my_task_id;
        /* get a unique task ID */
        #pragma omp atomic capture
        {
            my_task_id = total_tasks_created;
            total_tasks_created++;
        }

        /* Spawn new task only if limit is not reached */
        if (my_task_id < g_num_tasks) {
            /* Generate a new seed for the child task */
            unsigned long child_seed = my_rand(&seed, 0, 1000000000);
            #pragma omp task firstprivate(child_seed)
            pi_task(child_seed);
        } else {
            /* Stop spawning if max tasks reached */
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 6) {
        printf("Usage: %s num_tasks num_threads lower upper seed\n", argv[0]);
        return 1;
    }

    /* Parse command line arguments */
    g_num_tasks     = atoi(argv[1]);
    int num_threads = atoi(argv[2]);
    g_lower         = atol(argv[3]);
    g_upper         = atol(argv[4]);
    unsigned long seed = strtoul(argv[5], NULL, 10);

    /* Setup OpenMP environment */
    omp_set_num_threads(num_threads);
    tasks_per_thread = calloc(num_threads, sizeof(int));
    
    if (!tasks_per_thread) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    double start = omp_get_wtime();

    /* parallel region */
    #pragma omp parallel
    {
        /* Only one thread starts the recursion */
        #pragma omp single
        {
            #pragma omp atomic write
            total_tasks_created = 1;

            #pragma omp task firstprivate(seed)
            pi_task(seed);
        }
    }

    double end = omp_get_wtime();

    /* results */
    long total_executed = 0;
    for (int i = 0; i < num_threads; i++)
        total_executed += tasks_per_thread[i];

    double avg_pi = pi_sum / total_executed;

    /* Print */
    printf("Average pi: %.10f\n", avg_pi);
    for (int i = 0; i < num_threads; i++) {
        printf("Thread %d computed %d tasks\n", i, tasks_per_thread[i]);
    }
    printf("Execution took %.4f s\n", end - start);

    free(tasks_per_thread);
    return 0;
}