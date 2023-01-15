//
// Created by xuzaixiang on 2022/12/31.
//

#ifndef DATASTRUCTURE_DS_RHMAP_H
#define DATASTRUCTURE_DS_RHMAP_H

#ifndef DS_RHMAP_CAPACITY
#define DS_RHMAP_CAPACITY 0x10
#endif//DS_RHMAP_CAPACITY

#include <stddef.h>// size_t

// robin hood hash map
struct ds_rhmap;
struct ds_rhmap *ds_rhmap_new(size_t cap, size_t ksize, size_t vsize, size_t (*hash)(const void *), int (*compare)(const void *, const void *, void *), void *udata);
// @return NULL if failed,value if no key, not value if had same key
void *ds_rhmap_set(struct ds_rhmap *map, void *key, void *value);
void *ds_rhmap_get(struct ds_rhmap *map, void *key);
// @return -1 if NULL
size_t ds_rhmap_count(struct ds_rhmap *map);
void ds_rhmap_free(struct ds_rhmap **map);
int ds_rhmap_clear(struct ds_rhmap *map);
// @return old value
void *ds_rhmap_delete(struct ds_rhmap *map, void *key);
int ds_rhmap_foreach(struct ds_rhmap *map, int (*foreach)(void *key, void *value, void *udata, void *sdata), void *sdata);
void ds_rhmap_set_alloc(void *(*malloc)(size_t));
void ds_rhmap_set_free(void (*free)(void *));

#endif//DATASTRUCTURE_DS_RHMAP_H
