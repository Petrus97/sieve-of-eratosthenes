# Sieve of Eratosthenes

## Compile
This repo contains a single Makefile to compile different implementations of the sieve of Eratosthenens.
To compile the sequential, pthread and openMP version
```
make
```
This will generate three different binaries. Each binary will show its usage if tried to run them without arguments.
Binaries generated:
- eratosthenes
- eratosthenes_pthread
- eratosthenes_openmp

To compile MPI version we have two different commands:
- The collective version will use the Broadcast operation
```
make mpi
```
- The collective version will NOT use the broadcast operation:
```
make mpi-nobcast
```
Binaries generated:
- eratosthenes_mpi
- eratosthenes_mpi_collectiv
## Run
Sequential and OpenMP version
```
./eratosthenes <max-number>
and
./eratosthenes_openmmp <max-number>
```
Pthread version
```
./eratosthenes_pthread <max-number> <num-threads>
```
MPI version (locally)
```
mpiexec -np <number-of-processors> eratosthenes_mpi_collective <max-number>
```
MPI version (distributed)
```
mpiexec --hostfile hosts ./eratosthenes_mpi <max-number>
or
mpiexec --hostfile hosts ./eratosthenes_mpi_collective <max-number>
```
where `hosts` contains the computing nodes which to connect with ssh.