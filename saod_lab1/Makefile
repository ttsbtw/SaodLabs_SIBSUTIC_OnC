CC = cc
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic

.PHONY: all experiment
all: demo bench clean

demo: demo.c rbtree.c rbtree.h
	$(CC) $(CFLAGS) demo.c rbtree.c -o demo
bench: bench.c rbtree.c bstree.c rbtree.h bstree.h
	$(CC) $(CFLAGS) bench.c rbtree.c bstree.c -o bench
experiment: bench
	./bench > results.md
clean:
	rm -f demo
	rm -f bench
