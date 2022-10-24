#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>

/** The following defines has been taken from:
    Parallel programming in C with MPI and OpenMP
    Michael J. Quinn
    Chapter: 5.4.3 Block Decomposition Macros
    @param id rank of the process
    @param p number of processes
    @param n buffer dimension. Sieve up to n.
*/
#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id) + 1, p, n) - 1)
#define BLOCK_SIZE(id, p, n) (BLOCK_HIGH(id, p, n) - BLOCK_LOW(id, p, n) + 1)
#define BLOCK_OWNER(index, p, n) (((p) * ((index) + l) - l) / (n))

#define COMM_TAG 42
#define MASTER_NODE 0
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
    // Initialize MPI
    MPI_Init(&argc, &argv);
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int comm_size = 1;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    printf("Comm size: %d\n", comm_size);
    uint64_t max = 100;
    uint64_t sqrt_max = (uint64_t)sqrt(max);

    /* Create a list of natural numbers 1..Max
        0 (false) -> unmarked
        1 (true) -> marked
    */
    char *natural_numbers = (char *)calloc(max + 1, sizeof(char));
    memset(natural_numbers, false, max); // set all unmarked

    /*
        1) The master process allocates the array of prime numbers and calculate the sqrt of them.
    */
    uint64_t k = 2;
    while (square(k) <= sqrt_max)
    {
        mark(natural_numbers, max, k);
        find_smallest(natural_numbers, max, &k);
    }

    /*
        Calculation of the operating blocks for each process
    */
    uint64_t n = max - sqrt_max; // Remaining array
    uint64_t start = (sqrt_max + 1) + BLOCK_LOW(rank, comm_size, n);
    uint64_t end = (sqrt_max + 1) + BLOCK_HIGH(rank, comm_size, n);
    uint64_t blk_size = BLOCK_SIZE(rank, comm_size, n);

    printf("[%d] low: %ld high: %ld size: %ld\n", rank, start, end, blk_size);
    // Each process will build an array 
    int *tmp_array = (int *)calloc(blk_size, sizeof(int)); // tmp array to send // BUG Does not work with char or uint8 arrays
    // just to be sure
    memset(tmp_array, false, blk_size);
    /*
        make the calculation with the for loops
    */
    for (uint64_t j = 2; j <= sqrt_max; j++)
    {
        if (!natural_numbers[j]) // unmarked
        {
            for (uint64_t i = start; i <= end; i++)
            {
                if (i % j == 0)
                {
                    natural_numbers[i] = true;
                    tmp_array[i - start] = true;
                }
            }
        }
    }

    // Send the tmp array to the master node (that will concatenate)
    if (rank != MASTER_NODE)
    {
        // Send the array
        if(MPI_Send(tmp_array, blk_size, MPI_INT, MASTER_NODE, COMM_TAG, MPI_COMM_WORLD) == MPI_SUCCESS)
        {
            printf("[%d] success\n", rank);
        }
        

        free(tmp_array);
    }
    else // Master node
    {
        int dim;
        uint64_t start = sqrt_max + 1;
        // for (int id = 1; id < comm_size; id++)
        // {
        MPI_Status status;
        // Probe for an incoming message from slave process
        MPI_Probe(1, COMM_TAG, MPI_COMM_WORLD, &status);

        // When probe returns, the status object has the size and other
        // attributes of the incoming message. Get the message size
        MPI_Get_count(&status, MPI_INT, &dim);
        printf("dim %d\n", dim);

        // allocates the memory and receive the array
        int *tmp = (int *)calloc(blk_size, sizeof(int));
        // Receive the array in the allocated buffer
        MPI_Recv(tmp, dim, MPI_INT, 1, COMM_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Received array of %d size:\n", dim);
        for (size_t i = 0; i < dim; i++)
        {
            printf("[%ld]%d ", i + start + BLOCK_LOW(1, comm_size, n), (int)tmp[i]);
        }
        printf("\n");
        free(tmp);
        //}
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // if (rank == 0)
    //     print_primes(natural_numbers, max);

    free(natural_numbers);
    MPI_Finalize();
}
