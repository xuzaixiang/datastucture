#include "ds_rhmap.h"
#include <stdint.h>
#include <stdio.h>

size_t my_hash(const void *t) {
    return (size_t) t;
}
int my_compare(const void *a, const void *b, void *udata) {
    return *(int *) a == *(int *) b ? 0 : 1;
}
int foreach (void *key, void *value, void *, void *) {
    printf("\n%d", *(int *) key);
    printf("\n%d", *(int *) value);
    return 0;
}
int main() {
    struct ds_rhmap *map = ds_rhmap_new(16, 4, 4, my_hash, my_compare, NULL);
    int a = 1;
    int v = 2;
    ds_rhmap_set(map, &a, &v);
    ds_rhmap_foreach(map, foreach, NULL);
    void *ptr = ds_rhmap_get(map, &a);
    int r = *(int *) ptr;
    ptr = ds_rhmap_delete(map, &a);
    r = *(int *) ptr;
    ds_rhmap_clear(map);
    ptr = ds_rhmap_get(map, &a);
    return 0;
}
