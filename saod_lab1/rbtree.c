#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *copy_value(const char *s) {
    if (!s) s = "";
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (!p) { perror("malloc"); exit(EXIT_FAILURE); }
    memcpy(p, s, n);
    return p;
}
static enum rbcolor color(const struct rbtree *p) {
    return p ? p->color : RB_BLACK;
}
static void paint(struct rbtree *p, enum rbcolor c) { if (p) p->color = c; }

//left: правый ребёнок x поднимается на место x
static void rotate_left(struct rbtree **root, struct rbtree *x) {
    struct rbtree *y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) *root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}
//right наоборот
static void rotate_right(struct rbtree **root, struct rbtree *x) {
    struct rbtree *y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) *root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->right = x;
    x->parent = y;
}
struct rbtree *rbtree_lookup(struct rbtree *root, uint32_t key) {
    while (root && root->key != key)
        root = key < root->key ? root->left : root->right;
    return root;
}
struct rbtree *rbtree_min(struct rbtree *root) {
    if (root) while (root->left) root = root->left;
    return root;
}
struct rbtree *rbtree_max(struct rbtree *root) {
    if (root) while (root->right) root = root->right;
    return root;
}
struct rbtree *rbtree_add(struct rbtree *root, uint32_t key, char *value) {
    struct rbtree *p = NULL, *x = root;
    while (x) {
        p = x;
        if (key == x->key) {
            char *s = copy_value(value); //сначала копируем: value может быть x->value
            free(x->value); x->value = s;
            return root;
        }
        x = key < x->key ? x->left : x->right;
    }
    x = malloc(sizeof *x);
    if (!x) { perror("malloc"); exit(EXIT_FAILURE); }
    *x = (struct rbtree){key, copy_value(value), RB_RED, NULL, NULL, p};
    if (!p) root = x;
    else if (key < p->key) p->left = x;
    else p->right = x;
    while (x->parent && x->parent->color == RB_RED) { //решение конфликтов
        p = x->parent;
        struct rbtree *g = p->parent;
        if (p == g->left) {
            struct rbtree *u = g->right;
            if (color(u) == RB_RED) { //красный дядя: перенос конфликта вверх
                paint(p, RB_BLACK); paint(u, RB_BLACK); paint(g, RB_RED); x = g;
            } else {
                if (x == p->right) { x = p; rotate_left(&root, x); }
                paint(x->parent, RB_BLACK);
                paint(x->parent->parent, RB_RED);
                rotate_right(&root, x->parent->parent);
            }
        } else { // симметричные случаи
            struct rbtree *u = g->left;
            if (color(u) == RB_RED) {
                paint(p, RB_BLACK); paint(u, RB_BLACK); paint(g, RB_RED); x = g;
            } else {
                if (x == p->left) { x = p; rotate_right(&root, x); }
                paint(x->parent, RB_BLACK);
                paint(x->parent->parent, RB_RED);
                rotate_left(&root, x->parent->parent);
            }
        }
    }
    root->color = RB_BLACK;
    return root;
}
//замена позиций узла
static void transplant(struct rbtree **root, struct rbtree *u, struct rbtree *v) {
    if (!u->parent) *root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}
//родителя отдельно, потому что x может быть NULL
static void delete_fix(struct rbtree **root, struct rbtree *x, struct rbtree *p) {
    while (x != *root && color(x) == RB_BLACK) { //пока не дошли до корня и не перекрашиваем в красный
        if (x == p->left) {
            struct rbtree *w = p->right; //если х слева, его брат w
            if (color(w) == RB_RED) { //сделать брата чёрным
                paint(w, RB_BLACK); paint(p, RB_RED); rotate_left(root, p); w = p->right;
            }
            if (color(w ? w->left : NULL) == RB_BLACK &&
                color(w ? w->right : NULL) == RB_BLACK) { //дефицит вверх
                paint(w, RB_RED); x = p; p = x->parent;
            } else {
                if (color(w->right) == RB_BLACK) { //красный ближний
                    paint(w->left, RB_BLACK); paint(w, RB_RED);
                    rotate_right(root, w); w = p->right;
                }
                //красный дальний - последнее
                paint(w, color(p)); paint(p, RB_BLACK); paint(w->right, RB_BLACK);
                rotate_left(root, p); x = *root; p = NULL;
            }
        } else {
            struct rbtree *w = p->left;
            if (color(w) == RB_RED) {
                paint(w, RB_BLACK); paint(p, RB_RED); rotate_right(root, p); w = p->left;
            }
            if (color(w ? w->right : NULL) == RB_BLACK &&
                color(w ? w->left : NULL) == RB_BLACK) {
                paint(w, RB_RED); x = p; p = x->parent;
            } else {
                if (color(w->left) == RB_BLACK) {
                    paint(w->right, RB_BLACK); paint(w, RB_RED);
                    rotate_left(root, w); w = p->left;
                }
                paint(w, color(p)); paint(p, RB_BLACK); paint(w->left, RB_BLACK);
                rotate_right(root, p); x = *root; p = NULL;
            }
        }
    }
    paint(x, RB_BLACK);
}
struct rbtree *rbtree_delete(struct rbtree *root, uint32_t key) {
    struct rbtree *z = rbtree_lookup(root, key);
    if (!z) return root;
    struct rbtree *y = z, *x, *xp;
    enum rbcolor removed = y->color;
    //решение конфликтов
    if (!z->left) {
        x = z->right; xp = z->parent; transplant(&root, z, x);
    } else if (!z->right) {
        x = z->left; xp = z->parent; transplant(&root, z, x);
    } else {
        y = rbtree_min(z->right); removed = y->color; x = y->right;
        if (y->parent == z) {
            xp = y;
            if (x) x->parent = y;
        } else {
            xp = y->parent; transplant(&root, y, x);
            y->right = z->right; y->right->parent = y;
        }
        transplant(&root, z, y);
        y->left = z->left; y->left->parent = y; y->color = z->color;
    }
    free(z->value); free(z);
    if (removed == RB_BLACK) delete_fix(&root, x, xp); //восстанавлиаем после удалений
    return root;
}
void rbtree_free(struct rbtree *root) {
    if (!root) return;
    rbtree_free(root->left); rbtree_free(root->right);
    free(root->value); free(root);
}
static void rbtree_print_branch(struct rbtree *node,
                                const char *prefix,
                                int is_last)
{
    if (!node) return;

    printf("%s%s%" PRIu32 "(%c): %s\n",
           prefix,
           is_last ? "└── " : "├── ",
           node->key,
           node->color == RB_RED ? 'R' : 'B',
           node->value);

    char new_prefix[256];

    snprintf(new_prefix, sizeof(new_prefix),
             "%s%s",
             prefix,
             is_last ? "    " : "│   ");

    if (node->left && node->right) {
        rbtree_print_branch(node->left, new_prefix, 0);
        rbtree_print_branch(node->right, new_prefix, 1);
    }
    else if (node->left) {
        rbtree_print_branch(node->left, new_prefix, 1);
    }
    else if (node->right) {
        rbtree_print_branch(node->right, new_prefix, 1);
    }
}

void rbtree_print_dfs(struct rbtree *root, int level)
{
    (void)level;

    if (!root) return;

    printf("%" PRIu32 "(%c): %s\n",
           root->key,
           root->color == RB_RED ? 'R' : 'B',
           root->value);

    if (root->left && root->right) {
        rbtree_print_branch(root->left, "", 0);
        rbtree_print_branch(root->right, "", 1);
    }
    else if (root->left) {
        rbtree_print_branch(root->left, "", 1);
    }
    else if (root->right) {
        rbtree_print_branch(root->right, "", 1);
    }
}
