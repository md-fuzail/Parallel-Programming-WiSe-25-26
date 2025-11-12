#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <num_rows> <num_cols>\n", argv[0]);
        return 1;
    }

    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);

    if (rows <= 0 || cols <= 0)
    {
        printf("Error: both rows and columns must be positive integers.\n");
        return 1;
    }

    // Declare matrix and vectors
    int matrix[rows][cols];
    int vector[cols];
    int result[rows];

    // Initialize random number generator
    srand(42);

    // Initialize matrix and vector with random values
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            matrix[i][j] = rand() /RAND_MAX;
            printf("matrix[%d][%d] = %d\n", i, j, matrix[i][j]);
        }
    }

    for (int j = 0; j < cols; j++)
    {
        vector[j] = rand() /RAND_MAX;
        printf("vector[%d] = %d\n", j, vector[j]);
    }

    // Matrix-vector multiplication
    for (int i = 0; i < rows; i++)
    {
        double sum = 0.0;
        for (int j = 0; j < cols; j++)
        {
            sum += matrix[i][j] * vector[j];
        }
        result[i] = sum;
    }

    // Print result
    printf("\nMatrix-Vector Multiplication Result:\n");
    for (int i = 0; i < rows; i++)
    {
        printf("result[%d] = %.4f\n", i, result[i]);
    }

    return 0;
}