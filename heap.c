#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "heap.h"

static void *gc_stack_base = NULL;

char heap[HEAP_CAP] = {0};

Chunk_List alloced_chunks = {0};
Chunk_List freed_chunks = {
    .count = 1,
    .chunks = {
        [0] = {.start = heap, .size = sizeof(heap)},
    },
};
Chunk_List tmp_chunks = {0};

void chunk_list_insert(Chunk_List *list, void *start, size_t size)
{
    assert(list->count < CHUNK_LIST_CAP);
    list->chunks[list->count].start = start;
    list->chunks[list->count].size = size;

    char *chunk_start = start;
    for (size_t i = list->count; i > 0 && list->chunks[i - 1].start > chunk_start; i--) {
        const Chunk t = list->chunks[i];
        list->chunks[i] = list->chunks[i - 1];
        list->chunks[i - 1] = t;
    }

    list->count++;
}

void chunk_list_merge(Chunk_List *dst, const Chunk_List *src)
{
    dst->count = 0;
    for (size_t i = 0; i < src->count; i++) {
        const Chunk chunk = src->chunks[i];

        if (dst->count > 0) {
            Chunk *top_chunk = &dst->chunks[dst->count - 1];

            if (top_chunk->start + top_chunk->size == chunk.start) {
                top_chunk->size += chunk.size;
            } else {
                chunk_list_insert(dst, chunk.start, chunk.size);
            }
        } else {
            chunk_list_insert(dst, chunk.start, chunk.size);
        }
    }
}

void chunk_list_dump(const Chunk_List *list)
{
    printf("Chunks (%zu):\n", list->count);
    for (size_t i = 0; i < list->count; i++) {
        printf("start: %p, size: %zu\n",
               list->chunks[i].start,
               list->chunks[i].size);
    }
}

int chunk_list_find(const Chunk_List *list, void *ptr)
{
    for (size_t i = 0; i < list->count; i++) {
        if (list->chunks[i].start == ptr) {
            return (int) i;
        }
    }
    return -1;
}

void chunk_list_remove(Chunk_List *list, size_t index)
{
    assert(index < list->count);
    for (size_t i = index; i < list->count - 1; i++) {
        list->chunks[i] = list->chunks[i + 1];
    }
    list->count -= 1;
}

void *heap_alloc(size_t size)
{
    if (size > 0) {
        chunk_list_merge(&tmp_chunks, &freed_chunks);
        freed_chunks = tmp_chunks;

        for (size_t i = 0; i < freed_chunks.count; i++) {
            const Chunk chunk = freed_chunks.chunks[i];
            if (chunk.size >= size) {
                chunk_list_remove(&freed_chunks, i);

                const size_t tail_size = chunk.size - size;
                chunk_list_insert(&alloced_chunks, chunk.start, size);

                if (tail_size > 0) {
                    chunk_list_insert(&freed_chunks, chunk.start + size, tail_size);
                }

                return chunk.start;
            }
        }
    }

    return NULL;
}

void heap_free(void *ptr)
{
    if (ptr) {
        const int index = chunk_list_find(&alloced_chunks, ptr);
        assert(index >= 0);
        chunk_list_insert(&freed_chunks,
                          alloced_chunks.chunks[index].start,
                          alloced_chunks.chunks[index].size);
        chunk_list_remove(&alloced_chunks, (size_t) index);
    }
}

void heap_init(void *stack_base)
{
    gc_stack_base = stack_base;
}

static int chunk_list_find_containing(const Chunk_List *list, void *ptr)
{
    char *p = ptr;
    size_t lo = 0;
    size_t hi = list->count;

    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        char *start = list->chunks[mid].start;
        char *end = start + list->chunks[mid].size;
        if (p < start) {
            hi = mid;
        } else if (p >= end) {
            lo = mid + 1;
        } else {
            return (int) mid;
        }
    }

    return -1;
}

static void chunk_list_append(Chunk_List *list, void *start, size_t size)
{
    assert(list->count < CHUNK_LIST_CAP);
    list->chunks[list->count].start = start;
    list->chunks[list->count].size = size;
    list->count++;
}

static uintptr_t *align_word_up(void *p)
{
    uintptr_t x = (uintptr_t) p;
    x = (x + sizeof(uintptr_t) - 1) & ~(sizeof(uintptr_t) - 1);
    return (uintptr_t *) x;
}

static uintptr_t *align_word_down(void *p)
{
    uintptr_t x = (uintptr_t) p;
    x &= ~(sizeof(uintptr_t) - 1);
    return (uintptr_t *) x;
}

static void mark_region(Chunk_List *marked, void *from, void *to)
{
    char *start = from;
    char *end = to;
    if (start > end) {
        char *t = start;
        start = end;
        end = t;
    }

    uintptr_t heap_begin = (uintptr_t) heap;
    uintptr_t heap_end = heap_begin + HEAP_CAP;

    for (uintptr_t *p = align_word_up(start); p < align_word_down(end); p++) {
        uintptr_t value = *p;
        if (value < heap_begin || value >= heap_end) {
            continue;
        }

        int index = chunk_list_find_containing(&alloced_chunks, (void *) value);
        if (index < 0) {
            continue;
        }

        Chunk chunk = alloced_chunks.chunks[index];
        if (chunk_list_find(marked, chunk.start) < 0) {
            chunk_list_append(marked, chunk.start, chunk.size);
        }
    }
}

void heap_collect(void)
{
    Chunk_List marked = {0};
    uintptr_t stack_end;

    if (gc_stack_base != NULL) {
        mark_region(&marked, gc_stack_base, &stack_end);
    }

    for (size_t i = 0; i < marked.count; i++) {
        Chunk chunk = marked.chunks[i];
        mark_region(&marked, chunk.start, chunk.start + chunk.size);
    }

    size_t i = 0;
    while (i < alloced_chunks.count) {
        Chunk chunk = alloced_chunks.chunks[i];
        if (chunk_list_find(&marked, chunk.start) < 0) {
            chunk_list_insert(&freed_chunks, chunk.start, chunk.size);
            chunk_list_remove(&alloced_chunks, i);
        } else {
            i++;
        }
    }
}

void heap_reset(void)
{
    memset(heap, 0, sizeof(heap));
    alloced_chunks.count = 0;
    tmp_chunks.count = 0;
    freed_chunks.count = 1;
    freed_chunks.chunks[0].start = heap;
    freed_chunks.chunks[0].size = sizeof(heap);
}
