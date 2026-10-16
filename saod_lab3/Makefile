CC = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic

.PHONY: all experiment clean
all: demo bench

demo: demo.c minheap.c minheap.h fibheap.c fibheap.h
	$(CC) $(CFLAGS) demo.c minheap.c fibheap.c -o $@

bench: bench.c minheap.c minheap.h fibheap.c fibheap.h
	$(CC) $(CFLAGS) bench.c minheap.c fibheap.c -o $@

experiment: bench
	./bench > results.md.tmp && mv results.md.tmp results.md

clean:
	rm -f demo bench results.md.tmp
