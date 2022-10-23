#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>

/* The following defines has been taken from:
    Parallel programming in C with MPI and OpenMP
    Michael J. Quinn
    Chapter: 5.4.3 Block Decomposition Macros
*/
#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n) - 1)
#define BLOCK_SIZE(id,p,n) (BLOCK_HIGH(id,p,n) - BLOCK_LOW(id,p,n) + 1)
#define BLOCK_OWNER(index,p,n) (((p)*((index)+l)-l)/(n))

/**
 * @brief MPI implementation of the Sieve of Eratosthens
 *
 */

const char *TAG = "MPI";

void usage(void)
{
    printf("[%s] Usage:\n", TAG);
    printf("mpiexec --hostfile hosts ./eratosthenes_mpi MAX\n");
    printf("\tWhere MAX: u64 maximum number\n");
}

static inline uint64_t square(uint64_t k)
{
    return k * k;
}

void mark(char *n_numbers, uint64_t max, uint64_t k)
{
    uint64_t kk = square(k);
    for (uint64_t i = kk; i <= max; i++)
    {
        if (i % k == 0)
            n_numbers[i] = true; // mark
    }
}

void find_smallest(const char *n_numbers, uint64_t max, uint64_t *k)
{
    uint64_t i = (*k) + 1;
    while (i != max)
    {
        if (!n_numbers[i])
        { // unmarked
            *k = i;
            break;
        }
        i++;
    }
}

void print_primes(const char *n_numbers, uint64_t max)
{
    if (max <= 100)
    {
        for (uint64_t i = 0; i < max; i++)
        {
            if (!n_numbers[i])
                printf("%lu ", i);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    uint64_t max = 1000;
    int rank = 0;
    int comm_size = 1;
    // if (argc < 2)
    // {
    //     usage();
    //     exit(0);
    // }
    // if (argc == 2)
    // {
    //     max = strtoul(argv[1], NULL, 10);
    // }


    // Create a list of natural numbers 1..Max
    char *natural_numbers = (char *)calloc(max + 1, sizeof(char)); // Linux will not allocate the memory until it's used
    // 0 (false) -> unmarked
    // 1 (true) -> marked
    memset(natural_numbers, false, max); // set all unmarked

    uint64_t sqrt_max = (uint64_t)sqrt(max);
    int chunk = (max - sqrt_max) / comm_size;
    int remaining = max % comm_size;

    // BENCHMARK
    uint64_t k = 2;
    while (square(k) <= sqrt_max) // every process computes the prime numbers until sqrt_max
    {
        mark(natural_numbers, max, k);
        find_smallest(natural_numbers, max, &k);
    }
    /*
        1) The master process allocates the array of prime numbers and calculate the sqrt of them.
    */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    /*
        TODO: Calculate the operating block for each process
    */
    
    for (int id = 1; id < comm_size; id++) // send the array to all processes in the communication
    {
        MPI_Send(natural_numbers, max + 1, MPI_CHAR, id, MPI_ANY_TAG, MPI_COMM_WORLD);
    }
    /*
        TODO: make the calculation with the for loops
    */
//    for (uint64_t j = 2; j <= sqrt_max; j++)
//     {
//         if (!data->n_numbers[j]) // unmarked
//         {
//             for (uint64_t i = data->start; i <= (data->end > data->max ? data->max : data->end); i++)
//             {
//                 if (i % j == 0)
//                     data->n_numbers[i] = true;
//             }
//         }
//     }

    if(rank != 0) // I'm not the master
    {
        MPI_Send(natural_numbers, max, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD);
    }
    else // rank == 0 -> I'm the master node
    {
        for (int id = 1; id < comm_size; id++)
        {
            // Collect the results
            MPI_Recv(natural_numbers, max, MPI_CHAR, id, MPI_ANY_TAG, MPI_COMM_WORLD, NULL);
        }
    }

    free(natural_numbers);
    MPI_Finalize();
}
