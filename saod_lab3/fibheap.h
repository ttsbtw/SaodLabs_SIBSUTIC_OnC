#ifndef FIBHEAP_H
#define FIBHEAP_H

#include <stddef.h>

struct fibheap_node {
    int key;
    char *value;
    unsigned degree;
    int marked;
    struct fibheap_node *parent, *child, *left, *right;
};

struct fibheap {
    struct fibheap_node *min;
    size_t size;
};

struct fibheap *fibheap_create(void);
struct fibheap *fibheap_insert(struct fibheap *heap, int key, const char *value);
struct fibheap *fibheap_insert_node(struct fibheap *heap, int key, const char *value, struct fibheap_node **node_out);
struct fibheap_node *fibheap_min(struct fibheap *heap);
struct fibheap *fibheap_union(struct fibheap *heap1, struct fibheap *heap2);
struct fibheap *fibheap_extractmin(struct fibheap *heap);
struct fibheap *fibheap_decrease_key(struct fibheap *heap, struct fibheap_node *node, int newkey);
struct fibheap *fibheap_delete(struct fibheap *heap, int key);
void fibheap_free(struct fibheap *heap);

#endif
