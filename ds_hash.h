//
// Created by xuzaixiang on 2022/12/31.
//

#ifndef DATASTRUCTURE_DS_HASH_H
#define DATASTRUCTURE_DS_HASH_H

#include <stddef.h>
#include <stdint.h>

static inline uint32_t ds_hash_32(uint32_t val);
static inline uint64_t ds_hash_64(uint64_t val);
static inline size_t ds_hash_ptr(void *ptr);

// https://github.com/torvalds/linux/blob/master/include/linux/hash.h

#define GOLDEN_RATIO_32 0x61C88647
#define GOLDEN_RATIO_64 0x61C8864680B583EBull

static inline uint32_t ds_hash_32(uint32_t val) {
    return val * GOLDEN_RATIO_32;
}

static inline uint64_t ds_hash_64(uint64_t val) {
    return val * GOLDEN_RATIO_64;
}

//32 位系统：
//
//LP32 或 2/4/4 （ int 为 16 位， long 与指针为 32 位）
//Win16 API
//ILP32 或 4/4/4 （ int 、 long 及指针为 32 位）；
//Win32 API
//Unix 及类 Unix 系统（ Linux 、 Mac OS X ）
//64 位系统：
//
//LLP64 或 4/4/8 （ int 及 long 为 32 位，指针为 64 位）
//Win64 API
//LP64 或 4/8/8 （ int 为 32 位， long 及指针为 64 位）
//Unix 与类 Unix 系统（ Linux 、 Mac OS X ）

static inline size_t ds_hash_ptr(void *ptr) {
#if defined(__LP64__) || defined(_WIN64)
    return (size_t) ds_hash_64((uint64_t) ptr);
#else
    return (size_t) ds_hash_32((uint32_t) ptr);
#endif
}

#endif//DATASTRUCTURE_DS_HASH_H
