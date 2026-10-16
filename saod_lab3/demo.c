#include "fibheap.h"
#include "minheap.h"

#include <stdio.h>

int main(void)
{
    struct minheap *binary = minheap_create();
    struct fibheap *fibonacci = fibheap_create();
    if (!binary || !fibonacci) return 1;

    const int keys[] = {17, 4, 23, 9, 12};
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        char value[32];
        snprintf(value, sizeof(value), "item-%d", keys[i]);
        if (!minheap_insert(binary, keys[i], value) ||
            !fibheap_insert(fibonacci, keys[i], value)) return 1;
    }

    struct minheap_item *bmin = minheap_min(binary);
    struct fibheap_node *fmin = fibheap_min(fibonacci);
    printf("Минимумы: binary=%d (%s), fibonacci=%d (%s)\n",
           bmin->key, bmin->value, fmin->key, fmin->value);

    minheap_decrease_key(binary, 17, 2);
    fmin = fibheap_min(fibonacci);
    fibheap_decrease_key(fibonacci, fmin, 1);
    printf("После decrease-key: binary=%d, fibonacci=%d\n",
           minheap_min(binary)->key, fibheap_min(fibonacci)->key);

    minheap_extractmin(binary);
    fibheap_extractmin(fibonacci);
    printf("После extract-min: binary=%d, fibonacci=%d\n",
           minheap_min(binary)->key, fibheap_min(fibonacci)->key);
    minheap_delete(binary, 23);
    fibonacci = fibheap_delete(fibonacci, 23);
    printf("После удаления ключа 23: размеры binary=%zu, fibonacci=%zu\n",
           binary->size, fibonacci->size);

    struct minheap *binary_part = minheap_create();
    struct fibheap *fibonacci_part = fibheap_create();
    if (!binary_part || !fibonacci_part) return 1;
    minheap_insert(binary_part, 3, "from-second-heap");
    fibheap_insert(fibonacci_part, 3, "from-second-heap");
    binary = minheap_union(binary, binary_part);
    fibonacci = fibheap_union(fibonacci, fibonacci_part);
    printf("После union: binary=%d, fibonacci=%d\n",
           minheap_min(binary)->key, fibheap_min(fibonacci)->key);

    minheap_free(binary);
    fibheap_free(fibonacci);
    return 0;
}
