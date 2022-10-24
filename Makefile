CC=gcc
MPICC=mpicc
CFLAGS=-g -Wall -Werror -Wpedantic -DDEBUG=1 -fopenmp
LDFLAGS=-lm -lpthread

SRC_DIR=./src
INC_DIR=./include
OBJ_DIR=./objs

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

BINS := $(patsubst $(SRC_DIR)/%.c,%,$(SRCS))

all: $(BINS) # TODO mpi does not compile with gcc, need to remove from targets

release: CFLAGS=-O3 -Wall -Werror -Wpedantic -fopenmp
release: $(BINS)

mpi: CC:=$(MPICC)
mpi: $(BINS)

$(BINS): %: $(OBJ_DIR)/%.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -I$(INC_DIR)

$(OBJS): $(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INC_DIR)

check:
	cppcheck . -I $(INC_DIR)

clean:
	rm -rf $(BINS) $(OBJ_DIR)/*