#include <stdio.h>
#include <stdlib.h>

#include "heap.h"

int main(void)
{
    heap_init(__builtin_frame_address(0));
    heap_reset();
    srand(1);

    void *ptrs[128] = {0};
    for (int i = 0; i < 400; i++) {
        int slot = rand() % 128;
        if (ptrs[slot] != NULL) {
            heap_free(ptrs[slot]);
            ptrs[slot] = NULL;
        } else {
            size_t n = (size_t) (rand() % 200) + 1;
            ptrs[slot] = heap_alloc(n);
        }
    }

    printf("After 400 deterministic alloc/free operations:\n");
    Heap_Stats stats = heap_stats();
    heap_stats_print(&stats);

    for (int i = 0; i < 128; i++) {
        heap_free(ptrs[i]);
    }

    printf("After freeing remaining pointers (unmerged until next alloc):\n");
    stats = heap_stats();
    heap_stats_print(&stats);

    (void) heap_alloc(16);
    printf("After one alloc (free list coalesced):\n");
    stats = heap_stats();
    heap_stats_print(&stats);

    return 0;
}
