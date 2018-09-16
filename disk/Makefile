CC=gcc
CFLAGS=-g -Wall -O2 -lpthread

clean:
	$(RM) *.o MyDiskBench
	
all:clean
	$(CC) $(CFLAGS) -o MyDiskBench MyDiskBench.c -lpthread

run_all: all
	./MyDiskBench "disk-RS-100000-1thread.dat"

