CC=gcc
CFLAGS=-g -Wall -Werror -Wpedantic -DDEBUG=1
LDFLAGS=-lm

SRC_DIR=./src
INC_DIR=./include

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))

BIN=eratosthenes

all: $(BIN)

omp: CFLAGS+=-fopenmp
omp: $(BIN)

release: CFLAGS=-O3
release: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -I$(INC_DIR)
	rm -rf $(SRC_DIR)/*.o

test:
	echo $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I$(INC_DIR)

check:
	cppcheck . -I $(INC_DIR)

clean:
	rm -rf $(SRC_DIR)/*.o
	rm -rf $(BIN)