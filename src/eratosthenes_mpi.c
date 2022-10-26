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

const char *TAG = "MPI Send/Recv";

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
    else
    {
        int count = 0;
        for (uint64_t i = 0; i < max; i++)
        {
            if (!n_numbers[i])
                count++;
        }
        printf("prime count: %d\n", count);
    }
}

void print_array(int *buf, int dim, int rank)
{
    for (size_t i = 0; i < dim; i++)
    {
        printf("[%ld]%d ", i + rank, buf[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    double start_time, end_time;
    // Initialize MPI
    MPI_Init(&argc, &argv);
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int comm_size = 1;
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_ARE_FATAL); /* return info about errors */
    // Check processors by printing them
    char name[32];
    int len = 0;
    MPI_Get_processor_name(name, &len);
    printf("[%d] %s\n", rank, name);
    uint64_t max;
    if (argc < 2)
    {
        usage();
        if (rank == MASTER_NODE)
            MPI_Finalize();
        exit(0);
    }
    if (argc == 2)
    {
        max = strtoul(argv[1], NULL, 10);
    }
    // printf("%lu\n", max);
    uint64_t sqrt_max = (uint64_t)sqrt(max);

    /* Create a list of natural numbers 1..Max
        0 (false) -> unmarked
        1 (true) -> marked
    */
    char *natural_numbers = NULL;
    if (rank == MASTER_NODE) // Allocate all the array to collect the results later
    {
        natural_numbers = (char *)malloc((max + 1) * sizeof(char));
        memset(natural_numbers, false, max + 1);
    }
    else // slave nodes only need the first numbers computed
    {
        natural_numbers = (char *)malloc((sqrt_max + 1) * sizeof(char));
        memset(natural_numbers, false, sqrt_max + 1);
    }
    if (natural_numbers == NULL)
    {
        printf("[%d] Error allocating memory\n", rank);
    }
    // Wait everyone is ready
    MPI_Barrier(MPI_COMM_WORLD);
    /**
     * Start Benchmark
     */
    start_time = MPI_Wtime();
    // print_array(natural_numbers, max+1, rank);

    /*
        1) Each process allocates the array of prime numbers and calculate the sqrt of them.
    */
    uint64_t k = 2;
    while (square(k) <= sqrt_max)
    {
        mark(natural_numbers, sqrt_max, k);
        find_smallest(natural_numbers, sqrt_max, &k);
    }

    /*
        Calculation of the operating blocks for each process
    */
    uint64_t n = max - sqrt_max; // Remaining array
    uint64_t start = (sqrt_max + 1) + BLOCK_LOW(rank, comm_size, n);
    uint64_t end = (sqrt_max + 1) + BLOCK_HIGH(rank, comm_size, n);
    uint64_t blk_size = BLOCK_SIZE(rank, comm_size, n);

    // printf("[%d] low: %ld high: %ld size: %ld\n", rank, start, end, blk_size);

    // Send the tmp array to the master node (that will concatenate)
    if (rank != MASTER_NODE)
    {
        char *tmp_array = (char *)malloc(blk_size * sizeof(char)); // tmp array to send
        memset(tmp_array, false, blk_size * sizeof(char));
        if (tmp_array == NULL)
        {
            printf("Error allocate memory!");
        }
        // Each process calculate its prime numbers
        for (uint64_t j = 2; j <= sqrt_max; j++)
        {
            if (!natural_numbers[j]) // unmarked
            {
                for (uint64_t i = start; i <= end; i++)
                {
                    if (i % j == 0)
                    {
                        tmp_array[i - start] = true;
                    }
                }
            }
        }
        // Send the array
        if (MPI_Send(tmp_array, blk_size, MPI_BYTE, MASTER_NODE, COMM_TAG, MPI_COMM_WORLD) != MPI_SUCCESS)
        {
            printf("[%d] failed to send to Master Node\n", rank);
        }
        free(tmp_array);
    }
    else // Master node
    {
        // Master process calculates its prima numbers
        for (uint64_t j = 2; j <= sqrt_max; j++)
        {
            if (!natural_numbers[j]) // unmarked
            {
                for (uint64_t i = start; i <= end; i++)
                {
                    if (i % j == 0)
                    {
                        natural_numbers[i] = true;
                    }
                }
            }
        }
        // Retrieve the value calculated from the other processes
        for (int id = 1; id < comm_size; id++)
        {
            // printf("[%d] receiving from %d...\n", rank, id);
            uint64_t start_idx = (sqrt_max + 1) + BLOCK_LOW(id, comm_size, n);
            int dim;
            MPI_Status status;
            // Probe for an incoming message from slave process (blocking call)
            int ret = MPI_Probe(id, COMM_TAG, MPI_COMM_WORLD, &status);
            if (ret != MPI_SUCCESS)
                printf("Failed probing\n");
            // Get the message size stored inside the status parameter
            ret = MPI_Get_count(&status, MPI_BYTE, &dim); // The datatype param is the type of counted data (i.e. MPI_INT 1 = 4 byte)
            if (ret != MPI_SUCCESS)
                printf("Failed getting count\n");

            // Overwrite the array
            MPI_Recv(natural_numbers + start_idx, dim, MPI_BYTE, id, COMM_TAG, MPI_COMM_WORLD, &status);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    if (rank == 0)
    {
        printf("Elapsed %f\n", end_time - start_time);
        print_primes(natural_numbers, max);
    }

    free(natural_numbers);
    MPI_Finalize();
}
