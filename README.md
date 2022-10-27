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

### References
Slides provided by the course <b>Introduction to Parallel Programming</b> (1DL530) - Uppsala University<br>
[OpenMP introduction](https://www.youtube.com/watch?v=nE-xN4Bf8XI&list=PLLX-Q6B8xqZ8n8bwjGdzBJ25X2utwnoEG)<br>
Quinn, Michael J. Parallel Programming in C with MPI and OpenMP. Boston [etc.: McGraw-Hill Higher Education, 2003. Print.<br>
https://rookiehpc.github.io/mpi/docs/<br>
https://www.open-mpi.org/doc/v4.1<br>