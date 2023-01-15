//
// Created by xuzaixiang on 2023/1/2.
//

#include "ds_rhmap.h"
#include <stdlib.h>
#include <string.h>

static void *(*s_ds_rhmap_malloc)(size_t) = malloc;
static void (*s_ds_rhmap_free)(void *) = free;

typedef struct ds_rhmap_bucket {
    size_t hash;
    size_t dib;
} ds_rhmap_bucket_t;

typedef struct ds_rhmap {
    size_t ksize;
    size_t vsize;
    size_t bucketsz;
    size_t nbucket;
    size_t mask;
    size_t count;

    void *buckets;// bucket data
    void *spare;  // spare data
    void *edata;  // entry data
    void *udata;  // user data
    size_t growat;
    size_t shrinkat;

    size_t (*hash)(const void *);
    int (*compare)(const void *, const void *, void *);
} ds_rhmap_t;

void ds_rhmap_set_alloc(void *(*malloc)(size_t)) { s_ds_rhmap_malloc = malloc; }
void ds_rhmap_set_free(void (*free)(void *)) { s_ds_rhmap_free = free; }

static inline ds_rhmap_bucket_t *ds_rhmap_bucket_at(ds_rhmap_t *map, size_t index) {
    return (ds_rhmap_bucket_t *) (((char *) map->buckets) + (map->bucketsz * index));
}

static inline void *ds_rhmap_bucket_value(ds_rhmap_bucket_t *entry) {
    return ((char *) entry) + sizeof(ds_rhmap_bucket_t);
}

static inline size_t ds_rhmap_grow(size_t size) {
    return (size >> 2) * 3;// 0.75
}

static inline size_t ds_rhmap_shrink(size_t size) {
    return size / 10;// 0.1
}

static int ds_rhmap_resize(struct ds_rhmap *map, size_t new_cap) {
    ds_rhmap_t *new_map = ds_rhmap_new(new_cap, map->ksize, map->vsize, map->hash, map->compare, map->udata);
    if (new_map == NULL)
        return -1;
    for (size_t i = 0; i < map->nbucket; ++i) {
        ds_rhmap_bucket_t *entry = ds_rhmap_bucket_at(map, i);
        if (!entry->dib) {// null
            continue;
        }
        entry->dib = 1;// default
        size_t j = entry->hash & new_map->mask;
        for (;;) {
            ds_rhmap_bucket_t *bucket = ds_rhmap_bucket_at(new_map, j);
            if (bucket->dib == 0) {
                memcpy(bucket, entry, map->bucketsz);
                break;
            }
            if (bucket->dib < entry->dib) {
                memcpy(new_map->spare, bucket, map->bucketsz);
                memcpy(bucket, entry, map->bucketsz);
                memcpy(entry, new_map->spare, map->bucketsz);
            }
            j = (j + 1) & new_map->mask;
            entry->dib += 1;
        }
    }
    s_ds_rhmap_free(map->buckets);
    map->buckets = new_map->buckets;
    map->nbucket = new_map->nbucket;
    map->mask = new_map->mask;
    map->growat = new_map->growat;
    map->shrinkat = new_map->shrinkat;
    s_ds_rhmap_free(new_map);
    return 0;
}

struct ds_rhmap *ds_rhmap_new(size_t cap, size_t ksize, size_t vsize, size_t (*hash)(const void *), int (*compare)(const void *, const void *, void *), void *udata) {
    size_t ncap = DS_RHMAP_CAPACITY;
    if (cap < ncap) {
        cap = ncap;
    } else {
        while (ncap < cap) {
            ncap <<= 1;
        }
        cap = ncap;
    }
    size_t bucketsz = sizeof(ds_rhmap_bucket_t) + ksize + vsize;

    // map + spare + edata
    ds_rhmap_t *map = s_ds_rhmap_malloc(sizeof(ds_rhmap_t) + (bucketsz << 1));
    if (!map) {
        return NULL;
    }
    memset(map, 0, sizeof(ds_rhmap_t));
    map->hash = hash;
    map->compare = compare;

    map->nbucket = cap;
    map->mask = map->nbucket - 1;
    map->bucketsz = bucketsz;
    map->ksize = ksize;
    map->vsize = vsize;
    map->growat = ds_rhmap_grow(map->nbucket);
    map->shrinkat = ds_rhmap_shrink(map->nbucket);

    map->spare = ((char *) map) + sizeof(ds_rhmap_t);
    map->edata = (char *) map->spare + bucketsz;
    map->udata = udata;
    map->buckets = s_ds_rhmap_malloc(bucketsz * map->nbucket);
    if (map->buckets == NULL) {
        s_ds_rhmap_free(map);
        return NULL;
    }
    memset(map->buckets, 0, bucketsz * map->nbucket);
    return map;
}

void *ds_rhmap_set(struct ds_rhmap *map, void *key, void *value) {
    if (value == NULL || key == NULL)
        return NULL;
    if (map->growat == map->count) {
        if (ds_rhmap_resize(map, map->nbucket << 1)) {
            return NULL;
        }
    }
    ds_rhmap_bucket_t *entry = map->edata;
    entry->hash = map->hash(key);
    entry->dib = 1;
    memcpy(ds_rhmap_bucket_value(entry), key, map->ksize);
    memcpy(ds_rhmap_bucket_value(entry) + map->ksize, value, map->vsize);
    size_t i = entry->hash & map->mask;
    for (;;) {
        ds_rhmap_bucket_t *bucket = ds_rhmap_bucket_at(map, i);
        if (bucket->dib == 0) {
            memcpy(bucket, entry, map->bucketsz);
            map->count++;
            return NULL;
        }
        if (entry->hash == bucket->hash && map->compare(ds_rhmap_bucket_value(entry), ds_rhmap_bucket_value(bucket), map->udata) == 0) {
            memcpy(map->spare, ds_rhmap_bucket_value(bucket) + map->ksize, map->vsize);
            memcpy(ds_rhmap_bucket_value(bucket), ds_rhmap_bucket_value(entry), map->ksize + map->vsize);
            return map->spare;
        }
        if (bucket->dib < entry->dib) {
            memcpy(map->spare, bucket, map->bucketsz);
            memcpy(bucket, entry, map->bucketsz);
            memcpy(entry, map->spare, map->bucketsz);
        }
        i = (i + 1) & map->mask;
        entry->dib += 1;
    }
}
void *ds_rhmap_get(struct ds_rhmap *map, void *key) {
    if (NULL == key) return NULL;
    size_t hash = map->hash(key);
    size_t i = hash & map->mask;
    for (;;) {
        ds_rhmap_bucket_t *bucket = ds_rhmap_bucket_at(map, i);
        if (0 == bucket->dib) return NULL;
        if (hash == bucket->hash && map->compare(key, ds_rhmap_bucket_value(bucket), map->udata) == 0) {
            return ds_rhmap_bucket_value(bucket) + map->ksize;
        }
        i = (i + 1) & map->mask;
    }
}
size_t ds_rhmap_count(struct ds_rhmap *map) {
    if (NULL == map) return -1;
    return map->count;
}
void ds_rhmap_free(struct ds_rhmap **map) {
    if (NULL == map || NULL == *map) return;
    s_ds_rhmap_free((*map)->buckets);
    s_ds_rhmap_free(*map);
    *map = NULL;
}
int ds_rhmap_clear(struct ds_rhmap *map) {
    if (NULL == map) return -1;
    map->count = 0;
    if (map->nbucket != DS_RHMAP_CAPACITY) {
        void *new_buckets = s_ds_rhmap_malloc(map->bucketsz * DS_RHMAP_CAPACITY);
        if (!new_buckets) return -1;
        s_ds_rhmap_free(map->buckets);
        map->buckets = new_buckets;
        map->nbucket = DS_RHMAP_CAPACITY;
    }
    memset(map->buckets, 0, map->bucketsz * map->nbucket);
    map->mask = map->nbucket - 1;
    map->growat = ds_rhmap_grow(map->nbucket);
    map->shrinkat = ds_rhmap_shrink(map->nbucket);
    return 0;
}
void *ds_rhmap_delete(struct ds_rhmap *map, void *key) {
    if (NULL == key || NULL == map) return NULL;
    size_t hash = map->hash(key);
    size_t i = hash & map->mask;
    for (;;) {
        ds_rhmap_bucket_t *bucket = ds_rhmap_bucket_at(map, i);
        if (0 == bucket->dib) return NULL;
        if (hash == bucket->hash && map->compare(key, ds_rhmap_bucket_value(bucket), map->udata) == 0) {
            memcpy(map->spare, ds_rhmap_bucket_value(bucket) + map->ksize, map->vsize);
            bucket->dib = 0;
            for (;;) {
                ds_rhmap_bucket_t *prev = bucket;
                i = (i + 1) & map->mask;
                bucket = ds_rhmap_bucket_at(map, i);
                if (bucket->dib <= 1) {
                    prev->dib = 0;
                    break;
                }
                memcpy(prev, bucket, map->bucketsz);
                prev->dib--;
            }
            map->count--;
            if (map->nbucket > DS_RHMAP_CAPACITY && map->count == map->shrinkat) {
                ds_rhmap_resize(map, map->nbucket >> 1);
            }
            return map->spare;
        }
        i = (i + 1) & map->mask;
    }
}
int ds_rhmap_foreach(struct ds_rhmap *map, int (*foreach)(void *key, void *value, void *udata, void *sdata), void *sdata) {
    for (size_t i = 0; i < map->nbucket; i++) {
        ds_rhmap_bucket_t *bucket = ds_rhmap_bucket_at(map, i);
        if (bucket->dib) {
            void *item = ds_rhmap_bucket_value(bucket);
            if (0 != foreach (item, (char *) item + map->ksize, map->udata, sdata)) {
                return -1;
            }
        }
    }
    return 0;
}