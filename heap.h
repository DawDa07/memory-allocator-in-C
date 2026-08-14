#ifndef HEAP_H_
#define HEAP_H_

#include <stddef.h>

#define HEAP_CAP 640000
#define CHUNK_LIST_CAP 1024

typedef struct {
    char *start;
    size_t size;
} Chunk;

typedef struct {
    size_t count;
    Chunk chunks[CHUNK_LIST_CAP];
} Chunk_List;

extern char heap[HEAP_CAP];
extern Chunk_List alloced_chunks;
extern Chunk_List freed_chunks;
extern Chunk_List tmp_chunks;

void chunk_list_insert(Chunk_List *list, void *start, size_t size);
void chunk_list_merge(Chunk_List *dst, const Chunk_List *src);
void chunk_list_dump(const Chunk_List *list);
int chunk_list_find(const Chunk_List *list, void *ptr);
void chunk_list_remove(Chunk_List *list, size_t index);

void *heap_alloc(size_t size);
void heap_free(void *ptr);
void heap_collect(void);
void heap_reset(void);

#endif
