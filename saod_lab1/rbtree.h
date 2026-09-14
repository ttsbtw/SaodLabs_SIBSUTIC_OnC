#ifndef RBTREE_H
#define RBTREE_H
#include <inttypes.h>

enum rbcolor { RB_BLACK, RB_RED };
struct rbtree { //пустое дерево/лист это NULL. строки копируются и принадлежат дереву
    uint32_t key;
    char *value;
    enum rbcolor color;
    struct rbtree *left, *right, *parent;
};
struct rbtree *rbtree_add(struct rbtree *root, uint32_t key, char *value);//после сохранить возвращённый корень
struct rbtree *rbtree_lookup(struct rbtree *root, uint32_t key);
struct rbtree *rbtree_delete(struct rbtree *root, uint32_t key);//и тут сохранить возвращённый корень
struct rbtree *rbtree_min(struct rbtree *root);
struct rbtree *rbtree_max(struct rbtree *root);
void rbtree_free(struct rbtree *root);
void rbtree_print_dfs(struct rbtree *root, int level);
#endif
