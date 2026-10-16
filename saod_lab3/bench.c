#define _POSIX_C_SOURCE 200809L
#include "fibheap.h"
#include "minheap.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define REPEATS 3
#define BATCH 1000
static uint32_t random_state = 20261016u;
static volatile uint64_t sink;

static uint32_t random32(void)
{
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

static int random_key(void)
{
    return 10000 + (int)(random32() % 190001u);
}

static double seconds(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}

static void make_keys(int *keys, size_t n)
{
    for (size_t i = 0; i < n; ++i) keys[i] = random_key();
}

static struct minheap *build_binary(const int *keys, size_t n, struct minheap_item **handles)
{
    struct minheap *heap = minheap_create();
    if (!heap) return NULL;
    for (size_t i = 0; i < n; ++i) {
        if (!minheap_insert_item(heap, keys[i], "value", handles ? &handles[i] : NULL)) {
            minheap_free(heap); return NULL;
        }
    }
    return heap;
}

static struct fibheap *build_fibonacci(const int *keys, size_t n, struct fibheap_node **handles)
{
    struct fibheap *heap = fibheap_create();
    if (!heap) return NULL;
    for (size_t i = 0; i < n; ++i) {
        if (!fibheap_insert_node(heap, keys[i], "value", handles ? &handles[i] : NULL)) {
            fibheap_free(heap); return NULL;
        }
    }
    return heap;
}

static double one_binary(const int *keys, size_t n, int decrease)
{
    struct minheap_item **handles = decrease ? malloc(n * sizeof(*handles)) : NULL;
    struct minheap *heap = build_binary(keys, n, handles);
    if (!heap || (decrease && !handles)) { minheap_free(heap); free(handles); exit(EXIT_FAILURE); }
    double start = seconds();
    for (size_t i = 0; i < BATCH; ++i) {
        if (decrease) minheap_decrease_item(heap, handles[i], keys[i] - 300001);
        else minheap_extractmin(heap);
    }
    double elapsed = (seconds() - start) / BATCH;
    sink += heap->size;
    minheap_free(heap);
    free(handles);
    return elapsed;
}

static double one_fibonacci(const int *keys, size_t n, int decrease)
{
    struct fibheap_node **handles = decrease ? malloc(n * sizeof(*handles)) : NULL;
    struct fibheap *heap = build_fibonacci(keys, n, handles);
    if (!heap || (decrease && !handles)) { fibheap_free(heap); free(handles); exit(EXIT_FAILURE); }
    double start = seconds();
    for (size_t i = 0; i < BATCH; ++i) {
        if (decrease) fibheap_decrease_key(heap, handles[i], keys[i] - 300001);
        else fibheap_extractmin(heap);
    }
    double elapsed = (seconds() - start) / BATCH;
    sink += heap->size;
    fibheap_free(heap);
    free(handles);
    return elapsed;
}

static double all_binary(const int *keys, size_t n, int decrease)
{
    struct minheap_item **handles = decrease ? malloc(n * sizeof(*handles)) : NULL;
    struct minheap *heap = build_binary(keys, n, handles);
    if (!heap || (decrease && !handles)) { minheap_free(heap); free(handles); exit(EXIT_FAILURE); }
    double start = seconds();
    if (decrease) {
        for (size_t i = 0; i < n; ++i)
            minheap_decrease_item(heap, handles[i], keys[i] - 300001);
    } else {
        while (heap->size) minheap_extractmin(heap);
    }
    double elapsed = seconds() - start;
    sink += heap->size;
    minheap_free(heap);
    free(handles);
    return elapsed;
}

static double all_fibonacci(const int *keys, size_t n, int decrease)
{
    struct fibheap_node **handles = decrease ? malloc(n * sizeof(*handles)) : NULL;
    struct fibheap *heap = build_fibonacci(keys, n, handles);
    if (!heap || (decrease && !handles)) { fibheap_free(heap); free(handles); exit(EXIT_FAILURE); }
    double start = seconds();
    if (decrease) {
        for (size_t i = 0; i < n; ++i)
            fibheap_decrease_key(heap, handles[i], keys[i] - 300001);
    } else {
        while (heap->size) fibheap_extractmin(heap);
    }
    double elapsed = seconds() - start;
    sink += heap->size;
    fibheap_free(heap);
    free(handles);
    return elapsed;
}

static double median(double *values, size_t n)
{
    for (size_t i = 1; i < n; ++i) {
        double x = values[i];
        size_t j = i;
        while (j && values[j - 1] > x) { values[j] = values[j - 1]; --j; }
        values[j] = x;
    }
    return values[n / 2];
}

int main(void)
{
    const size_t sizes[] = {50000, 100000, 150000, 200000};
    puts("# Таблица 1. Среднее время одной операции после заполнения\n");
    puts("Для уменьшения погрешности усреднены первые 1000 операций на заполненной куче.\n");
    puts("| Элементов | binary extract-min, с | fibonacci extract-min, с | binary decrease-key, с | fibonacci decrease-key, с |");
    puts("|---:|---:|---:|---:|---:|");
    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); ++s) {
        size_t n = sizes[s];
        int *keys = malloc(n * sizeof(*keys));
        if (!keys) return EXIT_FAILURE;
        make_keys(keys, n);
        double be[REPEATS], fe[REPEATS], bd[REPEATS], fd[REPEATS];
        for (size_t r = 0; r < REPEATS; ++r) {
            be[r] = one_binary(keys, n, 0); fe[r] = one_fibonacci(keys, n, 0);
            bd[r] = one_binary(keys, n, 1); fd[r] = one_fibonacci(keys, n, 1);
        }
        printf("| %zu | %.9f | %.9f | %.9f | %.9f |\n", n,
               median(be, REPEATS), median(fe, REPEATS), median(bd, REPEATS), median(fd, REPEATS));
        free(keys);
    }
    puts("\n# Таблица 2. Операция над всеми элементами\n");
    puts("| Элементов | binary extract-min, с | fibonacci extract-min, с | binary decrease-key, с | fibonacci decrease-key, с |");
    puts("|---:|---:|---:|---:|---:|");
    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); ++s) {
        size_t n = sizes[s];
        int *keys = malloc(n * sizeof(*keys));
        if (!keys) return EXIT_FAILURE;
        make_keys(keys, n);
        double be[REPEATS], fe[REPEATS], bd[REPEATS], fd[REPEATS];
        for (size_t r = 0; r < REPEATS; ++r) {
            be[r] = all_binary(keys, n, 0); fe[r] = all_fibonacci(keys, n, 0);
            bd[r] = all_binary(keys, n, 1); fd[r] = all_fibonacci(keys, n, 1);
        }
        printf("| %zu | %.6f | %.6f | %.6f | %.6f |\n", n,
               median(be, REPEATS), median(fe, REPEATS), median(bd, REPEATS), median(fd, REPEATS));
        free(keys);
    }
    return sink == UINT64_MAX ? EXIT_FAILURE : EXIT_SUCCESS;
}
