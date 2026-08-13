//54:30

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#define HEAP_CAP 640000
#define HEAP_ALLOCED_CAP 1024
#define HEAP_FREED_CAP 1024

typedef struct {
    void *start;
    size_t size;
} Chunk;

typedef struct {
    size_
    Heap_Chunk chunks[HEAP_ALLOCED_CAP];
} Chunk_List;

void chunk_list_dump(const Chunk_List *list){
    printf("Chunks (%zu):\n", list->size);
    for (size_t i = 0; i < list->size; i++){
        printf("start: %p, size: %zu\n", 
            list->chunks[i].start, 
            list->chunks[i].size);
    }
}

int chunk_list_find(const Chunk_List *list, void *ptr){
    assert(false);
    return -1;
}

void chunk_list_insert(Chunk_List *list, void *ptr, size_t size){
    assert(false);
}

void chunk_list_remove(Chunk_List *list, size_t index){
    assert(false);
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
    assert(false);
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

