#ifndef BSTREE_H
#define BSTREE_H
#include <inttypes.h>
struct bstree { uint32_t key; struct bstree *left, *right; };
struct bstree *bstree_add(struct bstree *root, uint32_t key);
struct bstree *bstree_lookup(struct bstree *root, uint32_t key);
struct bstree *bstree_max(struct bstree *root);
void bstree_free(struct bstree *root);
struct bstree *bstree_append_sorted(struct bstree *tail, uint32_t key);
#endif
