#ifndef RTREE_H
#define RTREE_H

#include <stdbool.h>
#include <stdint.h>

/* label — копия фрагмент кея, у корня он пустой. */
struct rtree {
    char *label;
    uint32_t value;
    bool terminal;
    struct rtree *child;
    struct rtree *next;
};

struct rtree *rtree_create(void);
struct rtree *rtree_insert(struct rtree *root, char *key, uint32_t value);
struct rtree *rtree_lookup(struct rtree *root, char *key);
struct rtree *rtree_delete(struct rtree *root, char *key);
void rtree_print(struct rtree *root, int level);
void rtree_free(struct rtree *root);

#endif
