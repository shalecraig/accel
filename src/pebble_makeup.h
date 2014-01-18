#ifndef ACCEL_PEBBLE_SUPPLEMENTARY
#define ACCEL_PEBBLE_SUPPLEMENTARY

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void *my_realloc(void *old_ptr, size_t new_size, size_t old_size) {
    if (new_size == old_size) {
        return old_ptr;
    }
    if (old_ptr == NULL && old_size != 0) {
        return NULL;
    }
    if (new_size == 0) {
        free(old_ptr);
        return malloc(new_size);
    }
    void *p = malloc(new_size);

    if (!p) return NULL;

    size_t min_size = new_size < old_size ? new_size : old_size;
    memcpy(p, old_ptr, min_size);
    free(old_ptr);
    return p;
}

void *my_calloc(size_t num, size_t size) {
    void * allocd = malloc(num * size);
    if (allocd == NULL) return allocd;
    memset(allocd, 0, num * size);
    return allocd;
}

#endif
