CC=gcc
CFLAGS=-g -Wall -O2 -lpthread

clean:
	$(RM) *.o MyNETBench-TCP MyNETBench-UDP

all:clean
	$(CC) $(CFLAGS) -o MyNETBench-TCP MyNETBench-TCP.c -pthread
	$(CC) $(CFLAGS) -o MyNETBench-UDP MyNETBench-UDP.c -pthread

