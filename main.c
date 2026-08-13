//54:30

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

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

void chunk_list_dump(const Chunk_List *list){
    printf("Chunks (%zu):\n", list->size);
    for (size_t i = 0; i < list->size; i++){
        printf("start: %p, size: %zu\n", 
            list->chunks[i].start, 
            list->chunks[i].size);
    }
}

int chunk_start_compar(const void *a, const void *b){
    const Chunk *a_chunk = a;
    const Chunk *b_chunk = b;
    return a_chunk->start - b_chunk->start;
}

int chunk_list_find(const Chunk_List *list, void *ptr){
    Chunk key = {.start = ptr};

    Chunk *result = bsearch(&key, list->chunks, 
                list->count, sizeof(list->chunks[0]), 
                chunk_start_compar);
    if (result != 0){
        assert(list->chunks <= result);
        return (result - list->chunks) / sizeof(list->chunks[0]);
    } else {
        return -1;
    }
}

void chunk_list_insert(Chunk_List *list, void *start, size_t size)
{
    assert(list->count < CHUNK_LIST_CAP);
    list->chunks[list->count].start = start;
    list->chunks[list->count].size = size;
    
    for (size_t i = list->count; i > 0 && list->chunks[i-1].start > start; i--){
        const Chunk t = list->chunks[i];
        list->chunks[i] = list->chunks[i-1];
        list->chunks[i-1] = t;
    }

    list->count++;
}

void chunk_list_remove(Chunk_List *list, size_t index){
    assert(idnex < list->count);
    for (size_t i = index; i < list->count - 1; i++){
        list->chunks[i] = list->chunks[i+1];
    }
    list->count -= 1;

}


char heap[HEAP_CAP] = {0};
size_t heap_size = 0;

Chunk_List alloced_chunks = {0};
Chunk_List freed_chunks = {0};


void *heap_alloc(size_t size)
{
    if (size > 0){
        assert(heap_size + size <= HEAP_CAP);
        void *ptr = heap + heap_size;
        heap_size += size;

        chunk_list_insert(&alloced_chunks, ptr, size);  
        return ptr;  
    } else {
        return NULL;
    }

   
}

void heap_free(void *ptr)
{
    if (ptr){
        const int index = chunk_list_find(&alloced_chunks, ptr);
        assert(index >= 0);
        chunk_list_insert(&freed_chunks, 
                          alloced_chunks.chunks[index].start, 
                          alloced_chunks.chunks[index].size);
        chunk_list_remove(&alloced_chunks, (size_t) index);
    }
    
}

void heap_collect()
{
    assert(false);
}

int main()
{
    char *root = heap_alloc(26);
    for (int i=0; i<26; i++){
        root[i] = 'A' + i;
    }

    heap_dump_alloced_chunks();

    heap_free(root);

    return 0;
}

