#ifndef DS_HASHMAP_H_
#define DS_HASHMAP_H_

#include <stddef.h>

/* Open-addressing int -> int map. One slot table grown with heap_realloc.
 * Call hashmap_free when done; GC is optional. */
typedef struct {
    int key;
    int value;
    unsigned char state; /* 0 empty, 1 occupied, 2 tombstone */
} HashMap_Slot;

typedef struct {
    HashMap_Slot *slots;
    size_t cap;
    size_t len;
    size_t tombstones;
} HashMap;

void hashmap_init(HashMap *m);
int hashmap_put(HashMap *m, int key, int value);
int hashmap_get(const HashMap *m, int key, int *out);
int hashmap_remove(HashMap *m, int key);
size_t hashmap_len(const HashMap *m);
void hashmap_free(HashMap *m);

#endif
