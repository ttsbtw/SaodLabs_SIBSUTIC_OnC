#ifndef MINHEAP_H
#define MINHEAP_H

#include <stddef.h>

struct minheap_item {
    int key;
    char *value;
    size_t index;
};

struct minheap {
    size_t size;
    size_t capacity;
    struct minheap_item **items;
};

struct minheap *minheap_create(void);
struct minheap *minheap_insert(struct minheap *heap, int key, const char *value);
struct minheap *minheap_insert_item(struct minheap *heap, int key, const char *value, struct minheap_item **item_out);
struct minheap_item *minheap_min(struct minheap *heap);
struct minheap *minheap_union(struct minheap *heap1, struct minheap *heap2);
struct minheap *minheap_extractmin(struct minheap *heap);
struct minheap *minheap_decrease_key(struct minheap *heap, int key, int newkey);
struct minheap *minheap_decrease_item(struct minheap *heap, struct minheap_item *item, int newkey);
struct minheap *minheap_delete(struct minheap *heap, int key);
void minheap_free(struct minheap *heap);

#endif
