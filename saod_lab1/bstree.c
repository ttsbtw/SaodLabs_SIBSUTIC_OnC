#include "bstree.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static struct bstree *node(uint32_t key) {
    struct bstree *p = malloc(sizeof *p);
    if (!p) { perror("malloc"); exit(EXIT_FAILURE); }
    *p = (struct bstree){key, NULL, NULL};
    return p;
}
struct bstree *bstree_add(struct bstree *root, uint32_t key) {
    struct bstree **p = &root;
    while (*p) {
        if ((*p)->key == key) return root;
        p = key < (*p)->key ? &(*p)->left : &(*p)->right;
    }
    *p = node(key);
    return root;
}
struct bstree *bstree_lookup(struct bstree *root, uint32_t key) {
    while (root && root->key != key) root = key < root->key ? root->left : root->right;
    return root;
}
struct bstree *bstree_max(struct bstree *root) {
    if (root) while (root->right) root = root->right;
    return root;
}
struct bstree *bstree_append_sorted(struct bstree *tail, uint32_t key) {
    assert(!tail || (!tail->right && key > tail->key));
    struct bstree *p = node(key);
    if (tail) tail->right = p;
    return p;
}
void bstree_free(struct bstree *root) {
    while (root) {
        if (root->left) {
            struct bstree *p = root->left;
            root->left = p->right; p->right = root; root = p;
        } else {
            struct bstree *p = root->right; free(root); root = p;
        }
    }
}
