#include "heap.h"

int main(void)
{
    char *root = heap_alloc(26);
    for (int i = 0; i < 26; i++) {
        root[i] = 'A' + i;
    }

    chunk_list_dump(&alloced_chunks);
    heap_free(root);

    return 0;
}
