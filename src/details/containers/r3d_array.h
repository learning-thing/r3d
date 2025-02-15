/*
 * Copyright (c) 2025 Le Juez Victor
 *
 * This software is provided "as-is", without any express or implied warranty. In no event
 * will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose, including commercial
 * applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not claim that you
 *   wrote the original software. If you use this software in a product, an acknowledgment
 *   in the product documentation would be appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *   as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#ifndef R3D_ARRAY_H
#define R3D_ARRAY_H

#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* Types definitions */

enum r3d_retcode_array {
    R3D_ARRAY_ERROR_OUT_OF_BOUNDS    = -2,
    R3D_ARRAY_ERROR_OUT_OF_MEMORY    = -1,
    R3D_ARRAY_SUCCESS                = 0,
    R3D_ARRAY_EMPTY                  = 1
};

typedef struct r3d_array_t {
    void *data;             // Pointer to array elements
    size_t count;           // Number of elements currently in the array
    size_t capacity;        // Total array capacity (allocated space)
    size_t elem_size;       // Size of an element (in bytes)
} r3d_array_t;

/* Function definitions */

static inline r3d_array_t
r3d_array_create(size_t capacity, size_t elem_size)
{
    r3d_array_t vec = { 0 };

    if (capacity == 0 || elem_size == 0) {
        return vec;
    }

    void *data = RL_MALLOC(capacity * elem_size);
    if (!data) return vec;

    vec.data = data;
    vec.capacity = capacity;
    vec.elem_size = elem_size;

    return vec;
}

static inline void
r3d_array_destroy(r3d_array_t* vec)
{
    if (vec->data) {
        RL_FREE(vec->data);
        vec->data = NULL;
    }
    vec->count = 0;
    vec->capacity = 0;
    vec->elem_size = 0;
}

static inline r3d_array_t
r3d_array_copy(const r3d_array_t* src)
{
    r3d_array_t vec = { 0 };

    size_t size_in_bytes = src->count * src->elem_size;
    if (size_in_bytes == 0) return vec;

    vec.data = RL_MALLOC(size_in_bytes);
    if (vec.data == NULL) return vec;

    memcpy(vec.data, src->data, size_in_bytes);

    vec.count = src->count;
    vec.capacity = src->count;
    vec.elem_size = src->elem_size;

    return vec;
}

static inline bool
r3d_array_is_valid(const r3d_array_t* vec)
{
    return vec->data != NULL
        && vec->capacity > 0
        && vec->elem_size > 0;
}

static inline bool
r3d_array_is_empty(const r3d_array_t* vec)
{
    return vec->count == 0;
}

static inline int
r3d_array_reserve(r3d_array_t* vec, size_t new_capacity)
{
    if (vec->capacity >= new_capacity) {
        return R3D_ARRAY_SUCCESS;
    }

    void *new_data = RL_REALLOC(vec->data, new_capacity * vec->elem_size);
    if (!new_data) return R3D_ARRAY_ERROR_OUT_OF_MEMORY;

    vec->data = new_data;
    vec->capacity = new_capacity;

    return R3D_ARRAY_SUCCESS;
}

static inline int
r3d_array_shrink_to_fit(r3d_array_t* vec)
{
    if (vec->count == vec->capacity) {
        return 1;
    }

    if (vec->count == 0) {
        return R3D_ARRAY_EMPTY;
    }

    void *new_data = RL_REALLOC(vec->data, vec->count * vec->elem_size);
    if (!new_data) return R3D_ARRAY_ERROR_OUT_OF_MEMORY;

    vec->data = new_data;
    vec->capacity = vec->count;

    return R3D_ARRAY_SUCCESS;
}

static inline void
r3d_array_clear(r3d_array_t* vec)
{
    vec->count = 0;
}

static inline void
r3d_array_fill(r3d_array_t* vec, const void* data)
{
    const void *end = (char*)vec->data + vec->capacity * vec->elem_size;
    for (char *ptr = (char*)vec->data; (void*)ptr < end; ptr += vec->elem_size) {
        memcpy(ptr, data, vec->elem_size);
    }
    vec->count = vec->capacity;
}

static inline int
r3d_array_insert(r3d_array_t* vec, size_t index, const void* elements, size_t count)
{
    if (index > vec->count) {
        return R3D_ARRAY_ERROR_OUT_OF_BOUNDS;
    }

    size_t new_size = vec->count + count;

    if (new_size > vec->capacity) {
        // Here we increase the capacity of the
        // array to the nearest power of two
        if ((new_size & (new_size - 1)) == 0) {
            new_size <<= 1; // *= 2
        } else {
            new_size--;
            new_size |= new_size >> 1;
            new_size |= new_size >> 2;
            new_size |= new_size >> 4;
            new_size |= new_size >> 8;
            new_size |= new_size >> 16;
            #if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
                new_size |= new_size >> 32;
            #endif
            new_size++;
        }
        int ret = r3d_array_reserve(vec, new_size);
        if (ret < 0) return ret;
    }

    // Moving items to make room for new items
    void *destination = (char*)vec->data + (index + count) * vec->elem_size;
    void *source = (char*)vec->data + index * vec->elem_size;
    size_t bytes_to_move = (vec->count - index) * vec->elem_size;
    memmove(destination, source, bytes_to_move);

    // Inserting new elements
    void *target = (char*)vec->data + index * vec->elem_size;
    memcpy(target, elements, count * vec->elem_size);

    // Updating array count
    vec->count = new_size;

    return R3D_ARRAY_SUCCESS;
}

static inline const void*
r3d_array_end(const r3d_array_t* vec)
{
    return (const char*)vec->data + vec->count * vec->elem_size;
}

static inline void*
r3d_array_back(r3d_array_t* vec)
{
    return (char*)vec->data + (vec->count - 1) * vec->elem_size;
}

static inline void*
r3d_array_front(r3d_array_t* vec)
{
    return vec->data;
}

static inline void*
r3d_array_at(r3d_array_t* vec, size_t index)
{
    if (index >= vec->count) return NULL;
    return (char*)vec->data + index * vec->elem_size;
}

static inline int
r3d_array_push_back(r3d_array_t* vec, const void *element)
{
    if (vec->count >= vec->capacity) {
        // Here we increase the capacity of the
        // array to the nearest power of two
        size_t new_size = vec->count + 1;
        if ((new_size & (new_size - 1)) == 0) {
            new_size <<= 1; // *= 2
        } else {
            new_size--;
            new_size |= new_size >> 1;
            new_size |= new_size >> 2;
            new_size |= new_size >> 4;
            new_size |= new_size >> 8;
            new_size |= new_size >> 16;
            #if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
                new_size |= new_size >> 32;
            #endif
            new_size++;
        }
        int ret = r3d_array_reserve(vec, new_size);
        if (ret < 0) return ret;
    }

    void *target = (char*)vec->data + vec->count * vec->elem_size;
    if (element) memcpy(target, element, vec->elem_size);
    else memset(target, 0, vec->elem_size);
    vec->count++;

    return R3D_ARRAY_SUCCESS;
}

static inline int
r3d_array_push_front(r3d_array_t* vec, const void *element)
{
    if (vec->count >= vec->capacity) {
        // Here we increase the capacity of the
        // array to the nearest power of two
        size_t new_size = vec->count + 1;
        if ((new_size & (new_size - 1)) == 0) {
            new_size <<= 1; // *= 2
        } else {
            new_size--;
            new_size |= new_size >> 1;
            new_size |= new_size >> 2;
            new_size |= new_size >> 4;
            new_size |= new_size >> 8;
            new_size |= new_size >> 16;
            #if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
                new_size |= new_size >> 32;
            #endif
            new_size++;
        }
        int ret = r3d_array_reserve(vec, new_size);
        if (ret < 0) return ret;
    }

    // Move all existing items to the right to make room
    void *destination = (char*)vec->data + vec->elem_size;
    void *source = vec->data;
    size_t bytes_to_move = vec->count * vec->elem_size;
    memmove(destination, source, bytes_to_move);

    // Copy new item to start or fill with zeroes
    if (element) memcpy(vec->data, element, vec->elem_size);
    else memset(vec->data, 0, vec->elem_size);

    // Increment count
    vec->count++;

    return R3D_ARRAY_SUCCESS;
}

static inline int
r3d_array_push_at(r3d_array_t* vec, size_t index, const void* element)
{
    if (index >= vec->count) {
        return R3D_ARRAY_ERROR_OUT_OF_BOUNDS;
    }

    if (vec->count >= vec->capacity) {
        // Here we increase the capacity of the
        // array to the nearest power of two
        size_t new_size = vec->count + 1;
        if ((new_size & (new_size - 1)) == 0) {
            new_size <<= 1; // *= 2
        } else {
            new_size--;
            new_size |= new_size >> 1;
            new_size |= new_size >> 2;
            new_size |= new_size >> 4;
            new_size |= new_size >> 8;
            new_size |= new_size >> 16;
            #if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
                new_size |= new_size >> 32;
            #endif
            new_size++;
        }
        int ret = r3d_array_reserve(vec, new_size);
        if (ret < 0) return ret;
    }

    // Move existing items from index to make room
    void *destination = (char*)vec->data + index * vec->elem_size;
    void *source = (char*)vec->data + index * vec->elem_size;
    size_t bytes_to_move = (vec->count - index) * vec->elem_size;
    memmove(destination, source, bytes_to_move);
    
    // Copy new item to destination or fill with zeroes
    if (element) memcpy(source, element, vec->elem_size);
    else memset(source, 0, vec->elem_size);

    // Increment count
    vec->count++;

    return R3D_ARRAY_SUCCESS;
}

static inline int
r3d_array_pop_back(r3d_array_t* vec, void* element)
{
    if (vec->count == 0) {
        return R3D_ARRAY_EMPTY;
    }

    vec->count--;
    if (element != NULL) {
        void *source = (char*)vec->data + vec->count * vec->elem_size;
        memcpy(element, source, vec->elem_size);
    }

    return R3D_ARRAY_SUCCESS;
}

static inline int
r3d_array_pop_front(r3d_array_t* vec, void* element)
{
    if (vec->count == 0) {
        return R3D_ARRAY_EMPTY;
    }

    if (element != NULL) {
        memcpy(element, vec->data, vec->elem_size);
    }

    // Move all remaining items to the left
    void *source = (char*)vec->data + vec->elem_size;
    void *destination = vec->data;
    size_t bytes_to_move = (vec->count - 1) * vec->elem_size;
    memmove(destination, source, bytes_to_move);

    // Reduce array count
    vec->count--;

    return R3D_ARRAY_SUCCESS;
}

static inline int
r3d_array_pop_at(r3d_array_t* vec, size_t index, void* element)
{
    if (index >= vec->count) {
        return R3D_ARRAY_ERROR_OUT_OF_BOUNDS;
    }

    if (element != NULL) {
        void *source = (char*)vec->data + index * vec->elem_size;
        memcpy(element, source, vec->elem_size);
    }

    // Move the remaining items to the left to fill the hole
    void *destination = (char*)vec->data + index * vec->elem_size;
    void *source_start = (char*)vec->data + (index + 1) * vec->elem_size;
    size_t bytes_to_move = (vec->count - index - 1) * vec->elem_size;

    memmove(destination, source_start, bytes_to_move);

    // Reduce array count
    vec->count--;

    return R3D_ARRAY_SUCCESS;
}

static inline bool
r3d_array_compare(const r3d_array_t* a, const r3d_array_t* b)
{
    if (a->count != b->count || a->elem_size != b->elem_size) {
        return false;
    }

    return !memcmp(a->data, b->data, a->count * a->elem_size);
}

#endif // R3D_ARRAY_H