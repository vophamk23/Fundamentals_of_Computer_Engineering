CC = gcc
CFLAGS = -Wall -pthread -O2

all: seqlock test_multithread

seqlock: seqlock.c seqlock.h
	$(CC) $(CFLAGS) -o seqlock seqlock.c

test_multithread: test_multithread.c seqlock.h
	$(CC) $(CFLAGS) -o test_multithread test_multithread.c

run-simple: seqlock
	./seqlock

run-multi: test_multithread
	./test_multithread

clean:
	rm -f seqlock test_multithread

.PHONY: all run-simple run-multi clean