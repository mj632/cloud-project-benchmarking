CC = gcc
CFLAGS = -O3 -Wall -lpthread

clean:
	rm -rf *.o MyRAMBench

all: clean
	$(CC) $(CFLAGS) MyRAMBench.c -lm -pthread -o MyRAMBench
