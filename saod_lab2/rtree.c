#include "rtree.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *allocate(size_t size)
{
    void *p = malloc(size);
    if (!p) {
        fputs("rtree: недостаточно памяти\n", stderr);
        exit(EXIT_FAILURE);
    }
    return p;
}

static char *copy_part(const char *s, size_t length)
{
    char *copy = allocate(length + 1);
    memcpy(copy, s, length);
    copy[length] = '\0';
    return copy;
}

static struct rtree *new_node(const char *label, bool terminal, uint32_t value)
{
    struct rtree *node = allocate(sizeof(*node));
    node->label = copy_part(label, strlen(label));
    node->value = value;
    node->terminal = terminal;
    node->child = NULL;
    node->next = NULL;
    return node;
}

struct rtree *rtree_create(void)
{
    return new_node("", false, 0);
}

struct rtree *rtree_insert(struct rtree *root, char *key, uint32_t value) //O(m+hd) длина ключа+число посещенных * мощность
{
    if (!key) return root;
    if (!root) root = rtree_create();
    struct rtree *parent = root;
    const char *rest = key;
    while (*rest) {
        struct rtree *node = parent->child;
        while (node && node->label[0] != rest[0]) node = node->next;
        if (!node) { //1 случай подходящего ребенка нет
            node = new_node(rest, true, value);
            node->next = parent->child;
            parent->child = node;
            return root;
        }
        size_t common = 0;
        while (node->label[common] && rest[common] &&
               node->label[common] == rest[common]) ++common;
        if (node->label[common] == '\0') { //метка ребенка совпала целиком
            rest += common;
            parent = node;
            continue;
        }
        //старый узел становится общим префиксом, его хвост — ребёнком
        struct rtree *tail = new_node(node->label + common,
                                      node->terminal, node->value);
        tail->child = node->child;
        char *prefix = copy_part(node->label, common);
        free(node->label);
        node->label = prefix;
        node->child = tail;
        rest += common;
        node->terminal = (*rest == '\0');
        node->value = node->terminal ? value : 0;
        if (*rest) tail->next = new_node(rest, true, value);
        return root;
    }
    //полное совпадение: обновляем значение
    parent->terminal = true;
    parent->value = value;
    return root;
}

struct rtree *rtree_lookup(struct rtree *root, char *key) //O(m+hd)
{
    if (!root || !key) return NULL;
    struct rtree *node = root;
    const char *rest = key;
    while (*rest) {
        node = node->child;
        while (node && node->label[0] != rest[0]) node = node->next;
        if (!node) return NULL;
        size_t i = 0;
        while (node->label[i] && rest[i] && node->label[i] == rest[i]) ++i;
        if (node->label[i]) return NULL;
        rest += i;
    }
    return node->terminal ? node : NULL;
}

//link позволяет изменить указатель родителя или предыдущего соседа
static bool delete_at(struct rtree **link, const char *rest, bool is_root) //O(m+hd)
{
    struct rtree *node = *link;
    if (!*rest) {
        if (!node->terminal) return false;
        node->terminal = false;
        node->value = 0;
    } else {
        struct rtree **child_link = &node->child;
        while (*child_link && (*child_link)->label[0] != *rest)
            child_link = &(*child_link)->next;
        if (!*child_link) return false;
        const char *label = (*child_link)->label;
        size_t i = 0;
        while (label[i] && rest[i] && label[i] == rest[i]) ++i;
        if (label[i] || !delete_at(child_link, rest + i, false)) return false;
    }
    if (is_root || node->terminal) return true;
    if (!node->child) {
        *link = node->next;
        free(node->label);
        free(node);
    } else if (!node->child->next) {
        //нетерминальный узел с одним ребёнком сжимаем в одно ребро
        struct rtree *child = node->child;
        size_t a = strlen(node->label), b = strlen(child->label);
        char *joined = allocate(a + b + 1);
        memcpy(joined, node->label, a);
        memcpy(joined + a, child->label, b + 1);
        free(node->label);
        node->label = joined;
        node->terminal = child->terminal;
        node->value = child->value;
        node->child = child->child;
        free(child->label);
        free(child);
    }
    return true;
}

struct rtree *rtree_delete(struct rtree *root, char *key)
{
    if (root && key) (void)delete_at(&root, key, true);
    return root;
}

void rtree_print(struct rtree *root, int level)
{
    if (level < 0) level = 0;
    for (struct rtree *node = root; node; node = node->next) {
        for (int i = 0; i < level; ++i) fputs("  ", stdout);
        printf("\"%s\"", node->label);
        if (node->terminal) printf(" * = %" PRIu32, node->value);
        putchar('\n');
        rtree_print(node->child, level + 1);
    }
}

void rtree_free(struct rtree *root)
{
    while (root) {
        struct rtree *next = root->next;
        rtree_free(root->child);
        free(root->label);
        free(root);
        root = next;
    }
}
