CC=gcc
MPICC=mpicc
CFLAGS=-g -Wall -Werror -Wpedantic -DDEBUG=1 -fopenmp
LDFLAGS=-lm -lpthread

SRC_DIR=./src
INC_DIR=./include
OBJ_DIR=./objs

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

MPI_BIN=eratosthenes_mpi

BINS := $(filter-out $(MPI_BIN), $(patsubst $(SRC_DIR)/%.c,%,$(SRCS)))

#
# Sequential, Pthreads and OpenMP compilation
#
all: $(BINS)

$(BINS): %: $(OBJ_DIR)/%.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -I$(INC_DIR)

$(OBJS): $(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INC_DIR)

#
# MPI compilation
#
mpi: CC:=$(MPICC)
mpi: $(MPI_BIN)

$(MPI_BIN): %: $(OBJ_DIR)/%.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -I$(INC_DIR)


release: CFLAGS=-O3 -Wall -Werror -Wpedantic -fopenmp
release: $(BINS)

check:
	cppcheck . -I $(INC_DIR)

clean:
	rm -rf $(BINS) $(MPI_BIN) $(OBJ_DIR)/*