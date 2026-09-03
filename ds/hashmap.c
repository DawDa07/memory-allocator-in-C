#include "hashmap.h"

#include <stdint.h>
#include <string.h>

#include "heap.h"

enum {
    HASHMAP_INITIAL_CAP = 16,
    STATE_EMPTY = 0,
    STATE_OCCUPIED = 1,
    STATE_TOMBSTONE = 2,
};

#define HASHMAP_LOAD_NUM 3
#define HASHMAP_LOAD_DEN 4

void hashmap_init(HashMap *m)
{
    m->slots = NULL;
    m->cap = 0;
    m->len = 0;
    m->tombstones = 0;
}

static uint32_t hash_int(int key)
{
    uint32_t x = (uint32_t)key;
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

static size_t probe_index(const HashMap_Slot *slots, size_t cap, int key)
{
    size_t i = hash_int(key) & (cap - 1);
    size_t first_tomb = (size_t)-1;

    for (;;) {
        unsigned char st = slots[i].state;
        if (st == STATE_EMPTY) {
            return first_tomb != (size_t)-1 ? first_tomb : i;
        }
        if (st == STATE_TOMBSTONE) {
            if (first_tomb == (size_t)-1) {
                first_tomb = i;
            }
        } else if (slots[i].key == key) {
            return i;
        }
        i = (i + 1) & (cap - 1);
    }
}

static size_t find_index(const HashMap *m, int key)
{
    if (m->cap == 0) {
        return (size_t)-1;
    }
    size_t i = hash_int(key) & (m->cap - 1);
    for (;;) {
        unsigned char st = m->slots[i].state;
        if (st == STATE_EMPTY) {
            return (size_t)-1;
        }
        if (st == STATE_OCCUPIED && m->slots[i].key == key) {
            return i;
        }
        i = (i + 1) & (m->cap - 1);
    }
}

static int hashmap_rehash(HashMap *m, size_t new_cap)
{
    HashMap_Slot *old = m->slots;
    size_t old_cap = m->cap;

    HashMap_Slot *slots = heap_alloc(new_cap * sizeof(HashMap_Slot));
    if (slots == NULL) {
        return 0;
    }
    memset(slots, 0, new_cap * sizeof(HashMap_Slot));

    m->slots = slots;
    m->cap = new_cap;
    m->len = 0;
    m->tombstones = 0;

    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].state == STATE_OCCUPIED) {
            size_t j = probe_index(slots, new_cap, old[i].key);
            slots[j].key = old[i].key;
            slots[j].value = old[i].value;
            slots[j].state = STATE_OCCUPIED;
            m->len++;
        }
    }

    if (old != NULL) {
        heap_free(old);
    }
    return 1;
}

static int hashmap_ensure_room(HashMap *m)
{
    size_t used = m->len + m->tombstones;
    if (m->cap == 0) {
        return hashmap_rehash(m, HASHMAP_INITIAL_CAP);
    }
    if (used * HASHMAP_LOAD_DEN < m->cap * HASHMAP_LOAD_NUM) {
        return 1;
    }
    size_t new_cap = m->cap * 2;
    if (new_cap < m->cap) {
        return 0;
    }
    return hashmap_rehash(m, new_cap);
}

int hashmap_put(HashMap *m, int key, int value)
{
    if (!hashmap_ensure_room(m)) {
        return 0;
    }

    size_t i = probe_index(m->slots, m->cap, key);
    if (m->slots[i].state == STATE_OCCUPIED) {
        m->slots[i].value = value;
        return 1;
    }
    if (m->slots[i].state == STATE_TOMBSTONE) {
        m->tombstones--;
    }
    m->slots[i].key = key;
    m->slots[i].value = value;
    m->slots[i].state = STATE_OCCUPIED;
    m->len++;
    return 1;
}

int hashmap_get(const HashMap *m, int key, int *out)
{
    size_t i = find_index(m, key);
    if (i == (size_t)-1) {
        return 0;
    }
    if (out != NULL) {
        *out = m->slots[i].value;
    }
    return 1;
}

int hashmap_remove(HashMap *m, int key)
{
    size_t i = find_index(m, key);
    if (i == (size_t)-1) {
        return 0;
    }
    m->slots[i].state = STATE_TOMBSTONE;
    m->len--;
    m->tombstones++;
    return 1;
}

size_t hashmap_len(const HashMap *m)
{
    return m->len;
}

void hashmap_free(HashMap *m)
{
    if (m->slots != NULL) {
        heap_free(m->slots);
    }
    hashmap_init(m);
}
