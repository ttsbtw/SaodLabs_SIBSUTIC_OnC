#include "minheap.h"

#include <stdlib.h>
#include <string.h>

static char *copy_string(const char *s)
{
    size_t n = strlen(s) + 1;
    char *copy = malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

static void swap_items(struct minheap *heap, size_t a, size_t b)
{
    struct minheap_item *tmp = heap->items[a];
    heap->items[a] = heap->items[b];
    heap->items[b] = tmp;
    heap->items[a]->index = a;
    heap->items[b]->index = b;
}

static void sift_up(struct minheap *heap, size_t i)
{
    while (i > 0) {
        size_t parent = (i - 1) / 2;
        if (heap->items[parent]->key <= heap->items[i]->key) break;
        swap_items(heap, parent, i);
        i = parent;
    }
}

static void sift_down(struct minheap *heap, size_t i)
{
    for (;;) {
        size_t left = 2 * i + 1, right = left + 1, smallest = i;
        if (left < heap->size && heap->items[left]->key < heap->items[smallest]->key)
            smallest = left;
        if (right < heap->size && heap->items[right]->key < heap->items[smallest]->key)
            smallest = right;
        if (smallest == i) break;
        swap_items(heap, i, smallest);
        i = smallest;
    }
}

static int reserve(struct minheap *heap, size_t need)
{
    if (need <= heap->capacity) return 1;
    size_t cap = heap->capacity ? heap->capacity * 2 : 16;
    while (cap < need) cap *= 2;
    struct minheap_item **p = realloc(heap->items, cap * sizeof(*p));
    if (!p) return 0;
    heap->items = p;
    heap->capacity = cap;
    return 1;
}

struct minheap *minheap_create(void)
{
    return calloc(1, sizeof(struct minheap));
}

struct minheap *minheap_insert(struct minheap *heap, int key, const char *value)
{
    return minheap_insert_item(heap, key, value, NULL);
}

struct minheap *minheap_insert_item(struct minheap *heap, int key, const char *value, struct minheap_item **item_out)
{
    if (!heap || !value || !reserve(heap, heap->size + 1)) return NULL;
    struct minheap_item *item = calloc(1, sizeof(*item));
    if (!item) return NULL;
    item->value = copy_string(value);
    if (!item->value) { free(item); return NULL; }
    item->key = key;
    item->index = heap->size;
    heap->items[heap->size++] = item;
    sift_up(heap, item->index);
    if (item_out) *item_out = item;
    return heap;
}

struct minheap_item *minheap_min(struct minheap *heap)
{
    return heap && heap->size ? heap->items[0] : NULL;
}

struct minheap *minheap_union(struct minheap *a, struct minheap *b)
{
    if (!a || !b || a == b || b->size > (size_t)-1 - a->size) return NULL;
    if (!reserve(a, a->size + b->size)) return NULL;
    for (size_t i = 0; i < b->size; ++i) {
        b->items[i]->index = a->size;
        a->items[a->size++] = b->items[i];
    }
    free(b->items);
    free(b);
    for (size_t i = a->size / 2; i > 0; --i) sift_down(a, i - 1);
    return a;
}

static void remove_at(struct minheap *heap, size_t i)
{
    struct minheap_item *removed = heap->items[i];
    free(removed->value);
    free(removed);
    --heap->size;
    if (i != heap->size) {
        heap->items[i] = heap->items[heap->size];
        heap->items[i]->index = i;
        if (i && heap->items[i]->key < heap->items[(i - 1) / 2]->key) sift_up(heap, i);
        else sift_down(heap, i);
    }
}

struct minheap *minheap_extractmin(struct minheap *heap)
{
    if (heap && heap->size) remove_at(heap, 0);
    return heap;
}

struct minheap *minheap_decrease_item(struct minheap *heap, struct minheap_item *item, int newkey)
{
    if (!heap || !item || item->index >= heap->size || heap->items[item->index] != item || newkey > item->key)
        return heap;
    item->key = newkey;
    sift_up(heap, item->index);
    return heap;
}

struct minheap *minheap_decrease_key(struct minheap *heap, int key, int newkey)
{
    if (!heap || newkey > key) return heap;
    for (size_t i = 0; i < heap->size; ++i)
        if (heap->items[i]->key == key) return minheap_decrease_item(heap, heap->items[i], newkey);
    return heap;
}

struct minheap *minheap_delete(struct minheap *heap, int key)
{
    if (!heap) return NULL;
    for (size_t i = 0; i < heap->size; ++i)
        if (heap->items[i]->key == key) { remove_at(heap, i); break; }
    return heap;
}

void minheap_free(struct minheap *heap)
{
    if (!heap) return;
    for (size_t i = 0; i < heap->size; ++i) {
        free(heap->items[i]->value);
        free(heap->items[i]);
    }
    free(heap->items);
    free(heap);
}
