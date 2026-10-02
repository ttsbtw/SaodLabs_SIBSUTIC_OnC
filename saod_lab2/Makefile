CC = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic

.PHONY: all experiment clean
all: demo bench

demo: demo.c rtree.c rtree.h
	$(CC) $(CFLAGS) demo.c rtree.c -o $@

bench: bench.c rtree.c rtree.h
	$(CC) $(CFLAGS) bench.c rtree.c -o $@

experiment: bench
	./bench > results.md.tmp && mv results.md.tmp results.md

clean:
	rm -f demo bench results.md.tmp
