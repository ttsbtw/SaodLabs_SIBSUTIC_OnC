#include "rtree.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    struct rtree *root = rtree_create();
    char *keys[] = {"romane", "romanus", "romulus", "rubens", "ruber", "rubicon", "rubicundus", "roman"};
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        printf("\nВставка %s = %zu\n", keys[i], i + 1);
        root = rtree_insert(root, keys[i], (uint32_t)i + 1);
        rtree_print(root, 0);
    }
    root = rtree_insert(root, "roman", 1000);
    puts("\nПовторная вставка roman = 1000");
    rtree_print(root, 0);
    char *queries[] = {"roman", "rom", "rubicon", "missing"};
    for (size_t i = 0; i < sizeof(queries) / sizeof(queries[0]); ++i) {
        struct rtree *node = rtree_lookup(root, queries[i]);
        printf("Поиск %s: ", queries[i]);
        if (node) printf("найдено, значение = %" PRIu32 "\n", node->value);
        else puts("не найдено");
    }
    char *deleted[] = {"roman", "romane", "rubens", "missing", "romanus", "romulus", "ruber", "rubicon", "rubicundus"};
    for (size_t i = 0; i < sizeof(deleted) / sizeof(deleted[0]); ++i) {
        printf("\nУдаление %s\n", deleted[i]);
        root = rtree_delete(root, deleted[i]);
        rtree_print(root, 0);
    }
    rtree_free(root);
    return 0;
}
