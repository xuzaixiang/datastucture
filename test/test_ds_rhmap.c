#include "ds_rhmap.h"
#include <stdio.h>

size_t my_hash(const void *t) {
    return (size_t) t;
}
int my_compare(const void *a, const void *b, void *udata) {
    return *(int *) a == *(int *) b ? 0 : 1;
}
int my_foreach(void *key, void *value, void *) {
    printf("%d : %d\n", *(int *) key, *(int *) value);
    return 0;
}
int main() {
    struct ds_rhmap *map = ds_rhmap_new(4, 4, my_hash, my_compare, NULL);
    int a = 1;
    int v = 2;
    ds_rhmap_set(map, &a, &v);
    int v1 = 21;
    ds_rhmap_set(map, &a, &v1);
    ds_rhmap_set(map, &v1, &v);
    ds_rhmap_foreach(map, my_foreach);
    ds_rhmap_free(&map);
    return 0;
}
