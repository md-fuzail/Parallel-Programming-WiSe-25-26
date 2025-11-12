#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_iterations>\n", argv[0]);
        return 1;
    }

    long iterations = atol(argv[1]);

    if (iterations <= 0) {
        printf("Please provide a positive number of iterations.\n");
        return 1;
    }

    double pi = 0.0;
    double denominator = 1.0;

    for (long i = 0; i < iterations; i++) {
        if (i % 2 == 0)
            pi += 1.0 / denominator;
        else
            pi -= 1.0 / denominator;

        denominator += 2.0;
    }

    pi *= 4.0;

    printf("Approximation of Pi after %ld iterations: %.15f\n", iterations, pi);

    return 0;
}