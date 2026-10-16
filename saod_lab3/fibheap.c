#include "fibheap.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

static char *copy_string(const char *s)
{
    size_t n = strlen(s) + 1;
    char *copy = malloc(n);
    if (copy) memcpy(copy, s, n);
    return copy;
}

static void add_to_ring(struct fibheap_node **head, struct fibheap_node *node)
{
    if (!*head) {
        node->left = node->right = node;
        *head = node;
        return;
    }
    node->left = (*head)->left;
    node->right = *head;
    (*head)->left->right = node;
    (*head)->left = node;
}

static void remove_from_ring(struct fibheap_node **head, struct fibheap_node *node)
{
    if (node->right == node) *head = NULL;
    else {
        node->left->right = node->right;
        node->right->left = node->left;
        if (*head == node) *head = node->right;
    }
    node->left = node->right = node;
}

static void add_root(struct fibheap *heap, struct fibheap_node *node)
{
    node->parent = NULL;
    node->marked = 0;
    add_to_ring(&heap->min, node);
    if (heap->min == node || node->key < heap->min->key) heap->min = node;
}

struct fibheap *fibheap_create(void)
{
    return calloc(1, sizeof(struct fibheap));
}

struct fibheap *fibheap_insert(struct fibheap *heap, int key, const char *value)
{
    return fibheap_insert_node(heap, key, value, NULL);
}

struct fibheap *fibheap_insert_node(struct fibheap *heap, int key, const char *value, struct fibheap_node **node_out)
{
    if (!heap || !value) return NULL;
    struct fibheap_node *node = calloc(1, sizeof(*node));
    if (!node) return NULL;
    node->value = copy_string(value);
    if (!node->value) { free(node); return NULL; }
    node->key = key;
    node->left = node->right = node;
    add_root(heap, node);
    ++heap->size;
    if (node_out) *node_out = node;
    return heap;
}

struct fibheap_node *fibheap_min(struct fibheap *heap)
{
    return heap ? heap->min : NULL;
}

struct fibheap *fibheap_union(struct fibheap *a, struct fibheap *b)
{
    if (!a || !b || a == b || b->size > (size_t)-1 - a->size) return NULL;
    if (b->min) {
        if (!a->min) a->min = b->min;
        else {
            struct fibheap_node *ar = a->min->right;
            struct fibheap_node *bl = b->min->left;
            a->min->right = b->min;
            b->min->left = a->min;
            bl->right = ar;
            ar->left = bl;
            if (b->min->key < a->min->key) a->min = b->min;
        }
    }
    a->size += b->size;
    free(b);
    return a;
}

static void link_under(struct fibheap_node *child, struct fibheap_node *parent)
{
    struct fibheap_node *roots = NULL;
    child->left->right = child->right;
    child->right->left = child->left;
    child->left = child->right = child;
    add_to_ring(&roots, child);
    if (!parent->child) parent->child = child;
    else add_to_ring(&parent->child, child);
    child->parent = parent;
    child->marked = 0;
    ++parent->degree;
}

static void consolidate(struct fibheap *heap)
{
    struct fibheap_node *roots[128] = {0};
    if (!heap->min) return;
    size_t count = 1;
    for (struct fibheap_node *p = heap->min->right; p != heap->min; p = p->right) ++count;
    struct fibheap_node **list = malloc(count * sizeof(*list));
    if (!list) return;
    size_t n = 0;
    struct fibheap_node *p = heap->min;
    do { list[n++] = p; p = p->right; } while (p != heap->min);
    heap->min = NULL;
    for (size_t i = 0; i < n; ++i) {
        struct fibheap_node *x = list[i];
        if (x->parent) continue;
        if (x->left != x || x->right != x) {
            x->left->right = x->right;
            x->right->left = x->left;
            x->left = x->right = x;
        }
        unsigned d = x->degree;
        while (d < 128 && roots[d]) {
            struct fibheap_node *y = roots[d];
            roots[d] = NULL;
            if (y->key < x->key) { struct fibheap_node *tmp = x; x = y; y = tmp; }
            link_under(y, x);
            d = x->degree;
        }
        if (d < 128) roots[d] = x;
        else add_root(heap, x);
    }
    for (size_t i = 0; i < 128; ++i)
        if (roots[i]) add_root(heap, roots[i]);
    free(list);
}

struct fibheap *fibheap_extractmin(struct fibheap *heap)
{
    if (!heap || !heap->min) return heap;
    struct fibheap_node *z = heap->min;
    while (z->child) {
        struct fibheap_node *child = z->child;
        remove_from_ring(&z->child, child);
        --z->degree;
        add_root(heap, child);
    }
    remove_from_ring(&heap->min, z);
    --heap->size;
    free(z->value);
    free(z);
    if (heap->min) consolidate(heap);
    return heap;
}

static void cut(struct fibheap *heap, struct fibheap_node *node, struct fibheap_node *parent)
{
    remove_from_ring(&parent->child, node);
    --parent->degree;
    add_root(heap, node);
}

static void cascading_cut(struct fibheap *heap, struct fibheap_node *node)
{
    struct fibheap_node *parent = node->parent;
    if (!parent) return;
    if (!node->marked) node->marked = 1;
    else { cut(heap, node, parent); cascading_cut(heap, parent); }
}

struct fibheap *fibheap_decrease_key(struct fibheap *heap, struct fibheap_node *node, int newkey)
{
    if (!heap || !node || newkey > node->key) return heap;
    node->key = newkey;
    struct fibheap_node *parent = node->parent;
    if (parent && node->key < parent->key) {
        cut(heap, node, parent);
        cascading_cut(heap, parent);
    }
    if (!heap->min || node->key < heap->min->key) heap->min = node;
    return heap;
}

static struct fibheap_node *find_in_ring(struct fibheap_node *start, int key)
{
    if (!start) return NULL;
    struct fibheap_node *node = start;
    do {
        if (node->key == key) return node;
        struct fibheap_node *found = find_in_ring(node->child, key);
        if (found) return found;
        node = node->right;
    } while (node != start);
    return NULL;
}

struct fibheap *fibheap_delete(struct fibheap *heap, int key)
{
    if (!heap) return NULL;
    struct fibheap_node *node = find_in_ring(heap->min, key);
    if (!node) return heap;
    fibheap_decrease_key(heap, node, INT_MIN);
    /* If several nodes already have INT_MIN, remove the selected node directly. */
    if (heap->min != node) {
        if (node->parent) cut(heap, node, node->parent);
        heap->min = node;
    }
    return fibheap_extractmin(heap);
}

static void free_ring(struct fibheap_node *start)
{
    if (!start) return;
    struct fibheap_node *node = start;
    do {
        struct fibheap_node *next = node->right;
        free_ring(node->child);
        free(node->value);
        free(node);
        node = next;
    } while (node != start);
}

void fibheap_free(struct fibheap *heap)
{
    if (!heap) return;
    free_ring(heap->min);
    free(heap);
}
