#include "rbtree.h"
#include <stdio.h>
int main(void) {
    struct rbtree *root = NULL;
    uint32_t keys[] = {10, 20, 30, 15, 25, 5, 1};
    for (size_t i = 0; i < sizeof keys / sizeof keys[0]; ++i) {
        char value[32]; snprintf(value, sizeof value, "value-%" PRIu32, keys[i]);
        root = rbtree_add(root, keys[i], value);
        printf("\nПосле вставки %" PRIu32 ":\n", keys[i]); rbtree_print_dfs(root, 0);
    }
    struct rbtree *found = rbtree_lookup(root, 15);
    printf("\nПоиск 15: %s\n", found ? found->value : "не найден");
    printf("Минимум: %" PRIu32 "; максимум: %" PRIu32 "\n",
           rbtree_min(root)->key, rbtree_max(root)->key);
    root = rbtree_add(root, 15, "updated");
    printf("Обновление 15: %s\n", rbtree_lookup(root, 15)->value);
    uint32_t deleted[] = {1, 20, 10, 30, 5, 15, 25};
    for (size_t i = 0; i < sizeof deleted / sizeof deleted[0]; ++i) {
        root = rbtree_delete(root, deleted[i]);
        printf("\nПосле удаления %" PRIu32 ":\n", deleted[i]); rbtree_print_dfs(root, 0);
        if (!root) puts("(пустое дерево)");
    }
    rbtree_free(root);
    return 0;
}
