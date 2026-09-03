#include <stdio.h>

#include "heap.h"
#include "vec.h"

int main(void)
{
    heap_init(__builtin_frame_address(0));

    Vec v;
    vec_init(&v);

    printf("Pushing 1..20 onto a heap-backed vector.\n");
    for (int i = 1; i <= 20; i++) {
        if (!vec_push(&v, i)) {
            fprintf(stderr, "vec_push failed at %d\n", i);
            return 1;
        }
    }

    printf("len=%zu cap grows via heap_realloc; values:", vec_len(&v));
    for (size_t i = 0; i < vec_len(&v); i++) {
        int x = 0;
        vec_get(&v, i, &x);
        printf(" %d", x);
    }
    printf("\n");

    {
        Heap_Stats st = heap_stats();
        heap_stats_print(&st);
        printf("allocated chunks (expect 1 for the buffer): %zu\n", st.alloced_count);
    }

    vec_free(&v);
    {
        Heap_Stats st = heap_stats();
        printf("after vec_free: allocated chunks=%zu\n", st.alloced_count);
    }
    return 0;
}
