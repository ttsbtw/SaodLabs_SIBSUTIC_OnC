#define _POSIX_C_SOURCE 200809L
#include "rbtree.h"
#include "bstree.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const void *volatile sink;
static uint32_t state = UINT32_C(20260907);
static uint32_t random32(void) {
    state ^= state << 13; state ^= state >> 17; state ^= state << 5;
    return state;
}
static double now(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) { perror("clock_gettime"); exit(1); }
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}
static void call(int op, struct bstree *b, struct rbtree *r, uint32_t key) {
    switch (op) {
    case 0: sink = bstree_lookup(b, key); break;
    case 1: sink = rbtree_lookup(r, key); break;
    case 2: sink = bstree_max(b); break;
    default: sink = rbtree_max(r); break;
    }
}

static double measure(int op, struct bstree *b, struct rbtree *r, uint32_t key) {
    double samples[5];
    for (int i = 0; i < 100; ++i) call(op, b, r, key);
    for (int sample = 0; sample < 5; ++sample) {
        size_t count = 500; double elapsed;
        for (;;) {
            double start = now();
            for (size_t j = 0; j < count; ++j) call(op, b, r, key);
            elapsed = now() - start;
            if (elapsed >= 0.01) break;
            count *= 2;
        }
        samples[sample] = elapsed / (double)count;
    }
    for (int i = 1; i < 5; ++i)
        for (int j = i; j > 0 && samples[j] < samples[j-1]; --j) {
            double t = samples[j]; samples[j] = samples[j-1]; samples[j-1] = t;
        }
    return samples[2];
}
int main(void) {
    enum { RANGE = 990001, MAX_N = 200000 };
    uint32_t *keys = malloc((size_t)RANGE * sizeof *keys);
    if (!keys) { perror("malloc"); return 1; }
    for (uint32_t i = 0; i < RANGE; ++i) keys[i] = 10000 + i;
    //Фишер-Йетс
    for (uint32_t i = 0; i < MAX_N; ++i) {
        uint32_t j = i + random32() % (RANGE - i);
        uint32_t t = keys[i]; keys[i] = keys[j]; keys[j] = t;
    }
    double results[10][2][4];
    for (int scenario = 0; scenario < 2; ++scenario) {
        struct rbtree *r = NULL; struct bstree *b = NULL, *tail = NULL;
        for (uint32_t i = 0; i < MAX_N; ++i) {
            uint32_t key = scenario ? 10000 + i : keys[i];
            r = rbtree_add(r, key, "value");
            if (scenario) {
                tail = bstree_append_sorted(tail, key);
                if (!b) b = tail;
            } else b = bstree_add(b, key);
            if ((i + 1) % 20000 == 0) {
                int row = (int)((i + 1) / 20000 - 1);
                for (int op = 0; op < 4; ++op)
                    results[row][scenario][op] = measure(op, b, r, key);
                fprintf(stderr, "%s: %" PRIu32 " узлов — готово\n",
                        scenario ? "Возрастающие ключи" : "Случайные ключи", i + 1);
            }
        }
        rbtree_free(r); bstree_free(b);
    }
    free(keys);
    puts("# Результаты экспериментального исследования\n");
    puts("Время одной операции в секундах; медиана пяти серий, в каждой не менее 500 вызовов и 10 мс.\n");
    for (int table = 0; table < 2; ++table) {
        printf("## Таблица %d. %s\n\n", table + 1, table ? "Поиск максимума" : "Поиск последнего добавленного ключа");
        puts("| № | Узлов | BST, случайные | RBT, случайные | BST, возрастающие | RBT, возрастающие |");
        puts("|---:|---:|---:|---:|---:|---:|");
        for (int row = 0; row < 10; ++row)
            printf("| %d | %d | %.9e | %.9e | %.9e | %.9e |\n", row + 1, (row + 1) * 20000,
                   results[row][0][table * 2], results[row][0][table * 2 + 1],
                   results[row][1][table * 2], results[row][1][table * 2 + 1]);
        puts("");
    }
    return 0;
}
