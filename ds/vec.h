#ifndef DS_VEC_H_
#define DS_VEC_H_

#include <stddef.h>

/* Growable array of ints, allocated via heap_alloc / heap_realloc.
 * Call vec_free when done; GC is optional and not required for normal use. */
typedef struct {
    int *data;
    size_t len;
    size_t cap;
} Vec;

void vec_init(Vec *v);
int vec_push(Vec *v, int value);
int vec_pop(Vec *v, int *out);
int vec_get(const Vec *v, size_t index, int *out);
int vec_set(Vec *v, size_t index, int value);
size_t vec_len(const Vec *v);
void vec_free(Vec *v);

#endif
