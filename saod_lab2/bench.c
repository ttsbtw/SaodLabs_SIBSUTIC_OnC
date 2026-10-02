#define _POSIX_C_SOURCE 200809L
#include "rtree.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_KEYS 200000
#define QUERIES 100000
#define REPEATS 5

static uint32_t state = 20250902u;
static volatile uint64_t sink;

static uint32_t random32(void)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

static uint32_t bounded(uint32_t bound)
{
    uint32_t r, threshold = (uint32_t)(0u - bound) % bound;
    do { r = random32(); } while (r < threshold);
    return r % bound;
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

int main(void)
{
    char (*keys)[31] = malloc(MAX_KEYS * sizeof(*keys));
    char **queries = malloc(QUERIES * sizeof(*queries));
    if (!keys || !queries) {
        fputs("bench: недостаточно памяти\n", stderr);
        free(keys);
        free(queries);
        return EXIT_FAILURE;
    }
    struct rtree *root = rtree_create();
    puts("# Таблица 1. Время поиска в radix tree\n");
    puts("Ключи: уникальные случайные строки a–z длиной 6–30 байт. "
         "Seed: 20250902. Успешный поиск случайного существующего ключа.");
    puts("Для каждого размера: прогрев 100000 поисков, затем 5 серий по "
         "100000 поисков. Основной результат — медиана среднего времени одного поиска.\n");
    puts("| № | Количество элементов | rtree_lookup, с | Минимум серии, с/поиск | Максимум серии, с/поиск |");
    puts("|---:|---:|---:|---:|---:|");
    size_t count = 0;
    for (size_t n = 20000; n <= MAX_KEYS; n += 20000) {
        while (count < n) {
            size_t length = 6 + bounded(25);
            for (size_t j = 0; j < length; ++j)
                keys[count][j] = (char)('a' + bounded(26));
            keys[count][length] = '\0';
            if (rtree_lookup(root, keys[count])) continue;
            root = rtree_insert(root, keys[count], (uint32_t)count);
            ++count;
        }
        for (size_t j = 0; j < QUERIES; ++j) queries[j] = keys[bounded((uint32_t)n)];
        uint64_t expected = 0;
        for (size_t j = 0; j < QUERIES; ++j) {
            struct rtree *node = rtree_lookup(root, queries[j]);
            if (!node) { fputs("Ошибка поиска\n", stderr); return EXIT_FAILURE; }
            expected += node->value;
        }
        sink = expected;
        double times[REPEATS];
        for (size_t k = 0; k < REPEATS; ++k) {
            uint64_t checksum = 0;
            double start = seconds();
            for (size_t j = 0; j < QUERIES; ++j) {
                struct rtree *node = rtree_lookup(root, queries[j]);
                checksum += node->value;
            }
            times[k] = (seconds() - start) / QUERIES;
            sink = checksum;
            if (checksum != expected) { fputs("Ошибка контрольной суммы\n", stderr); return EXIT_FAILURE; }
        }
        for (size_t i = 1; i < REPEATS; ++i) {
            double value = times[i];
            size_t j = i;
            while (j && times[j - 1] > value) { times[j] = times[j - 1]; --j; }
            times[j] = value;
        }
        printf("| %zu | %zu | %.12f | %.12f | %.12f |\n", n / 20000, n,
               times[REPEATS / 2], times[0], times[REPEATS - 1]);
    }
    rtree_free(root);
    free(queries);
    free(keys);
    return 0;
}
