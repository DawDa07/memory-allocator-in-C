#include "vec.h"

#include <stddef.h>
#include <stdint.h>

#include "../heap.h"

enum { VEC_INITIAL_CAP = 8 };

void vec_init(Vec *v)
{
    v->data = NULL;
    v->len = 0;
    v->cap = 0;
}

static int vec_reserve(Vec *v, size_t need)
{
    if (need <= v->cap) {
        return 1;
    }

    size_t new_cap = v->cap == 0 ? VEC_INITIAL_CAP : v->cap;
    while (new_cap < need) {
        if (new_cap > (SIZE_MAX / 2) / sizeof(int)) {
            return 0;
        }
        new_cap *= 2;
    }

    int *grown = heap_realloc(v->data, new_cap * sizeof(int));
    if (grown == NULL) {
        return 0;
    }
    v->data = grown;
    v->cap = new_cap;
    return 1;
}

int vec_push(Vec *v, int value)
{
    if (!vec_reserve(v, v->len + 1)) {
        return 0;
    }
    v->data[v->len++] = value;
    return 1;
}

int vec_pop(Vec *v, int *out)
{
    if (v->len == 0) {
        return 0;
    }
    v->len--;
    if (out != NULL) {
        *out = v->data[v->len];
    }
    return 1;
}

int vec_get(const Vec *v, size_t index, int *out)
{
    if (index >= v->len || out == NULL) {
        return 0;
    }
    *out = v->data[index];
    return 1;
}

int vec_set(Vec *v, size_t index, int value)
{
    if (index >= v->len) {
        return 0;
    }
    v->data[index] = value;
    return 1;
}

size_t vec_len(const Vec *v)
{
    return v->len;
}

void vec_free(Vec *v)
{
    if (v->data != NULL) {
        heap_free(v->data);
    }
    v->data = NULL;
    v->len = 0;
    v->cap = 0;
}
