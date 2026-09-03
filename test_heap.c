#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "heap.h"

static int tests_run;
static int tests_failed;

#define EXPECT(cond) do { \
    tests_run++; \
    if (!(cond)) { \
        tests_failed++; \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static void test_chunk_list_insert_sorted(void)
{
    Chunk_List list = {0};

    chunk_list_insert(&list, heap + 100, 10);
    chunk_list_insert(&list, heap + 0, 20);
    chunk_list_insert(&list, heap + 50, 5);

    EXPECT(list.count == 3);
    EXPECT(list.chunks[0].start == heap + 0);
    EXPECT(list.chunks[0].size == 20);
    EXPECT(list.chunks[1].start == heap + 50);
    EXPECT(list.chunks[1].size == 5);
    EXPECT(list.chunks[2].start == heap + 100);
    EXPECT(list.chunks[2].size == 10);
}

static void test_chunk_list_find(void)
{
    Chunk_List list = {0};

    EXPECT(chunk_list_find(&list, heap) == -1);

    chunk_list_insert(&list, heap + 10, 4);
    chunk_list_insert(&list, heap + 20, 8);

    EXPECT(chunk_list_find(&list, heap + 10) == 0);
    EXPECT(chunk_list_find(&list, heap + 20) == 1);
    EXPECT(chunk_list_find(&list, heap + 99) == -1);
}

static void test_chunk_list_remove(void)
{
    Chunk_List list = {0};

    chunk_list_insert(&list, heap + 0, 1);
    chunk_list_insert(&list, heap + 10, 2);
    chunk_list_insert(&list, heap + 20, 3);

    chunk_list_remove(&list, 1);
    EXPECT(list.count == 2);
    EXPECT(list.chunks[0].start == heap + 0);
    EXPECT(list.chunks[1].start == heap + 20);

    chunk_list_remove(&list, 1);
    EXPECT(list.count == 1);
    EXPECT(list.chunks[0].start == heap + 0);

    chunk_list_remove(&list, 0);
    EXPECT(list.count == 0);
}

static void test_chunk_list_merge(void)
{
    Chunk_List src = {0};
    Chunk_List dst = {0};

    chunk_list_merge(&dst, &src);
    EXPECT(dst.count == 0);

    chunk_list_insert(&src, heap + 0, 10);
    chunk_list_merge(&dst, &src);
    EXPECT(dst.count == 1);
    EXPECT(dst.chunks[0].size == 10);

    chunk_list_insert(&src, heap + 10, 15);
    chunk_list_insert(&src, heap + 40, 5);
    chunk_list_merge(&dst, &src);
    EXPECT(dst.count == 2);
    EXPECT(dst.chunks[0].start == heap + 0);
    EXPECT(dst.chunks[0].size == 25);
    EXPECT(dst.chunks[1].start == heap + 40);
    EXPECT(dst.chunks[1].size == 5);
}

static void test_chunk_list_dump(void)
{
    Chunk_List list = {0};
    chunk_list_dump(&list);
    chunk_list_insert(&list, heap, 8);
    chunk_list_dump(&list);
}

static void test_heap_alloc_zero_and_oom(void)
{
    heap_reset();

    EXPECT(heap_alloc(0) == NULL);
    EXPECT(heap_alloc(HEAP_CAP + 1) == NULL);

    void *all = heap_alloc(HEAP_CAP);
    EXPECT(all == heap);
    EXPECT(alloced_chunks.count == 1);
    EXPECT(freed_chunks.count == 0);
    EXPECT(heap_alloc(1) == NULL);

    heap_free(all);
}

static void test_heap_alloc_split_and_write(void)
{
    heap_reset();

    char *p = heap_alloc(26);
    EXPECT(p == heap);
    EXPECT(alloced_chunks.count == 1);
    EXPECT(alloced_chunks.chunks[0].size == 32);
    EXPECT(freed_chunks.count == 1);
    EXPECT(freed_chunks.chunks[0].start == heap + 32);
    EXPECT(freed_chunks.chunks[0].size == HEAP_CAP - 32);

    for (int i = 0; i < 26; i++) {
        p[i] = (char) ('A' + i);
    }
    EXPECT(p[0] == 'A');
    EXPECT(p[25] == 'Z');
}

static void test_heap_free_null_and_coalesce(void)
{
    heap_reset();

    heap_free(NULL);

    void *a = heap_alloc(32);
    void *b = heap_alloc(32);
    void *c = heap_alloc(32);
    EXPECT(a != NULL && b != NULL && c != NULL);

    heap_free(a);
    heap_free(b);

    void *joined = heap_alloc(60);
    EXPECT(joined == a);
    EXPECT(alloced_chunks.count == 2);

    heap_free(joined);
    heap_free(c);
}

static void test_heap_first_fit_skips_small_hole(void)
{
    heap_reset();

    void *a = heap_alloc(32);
    void *b = heap_alloc(32);
    void *c = heap_alloc(64);

    heap_free(a);

    void *big = heap_alloc(48);
    EXPECT(big != a);
    EXPECT(big == (char *) c + 64);

    heap_free(b);
    heap_free(big);
    heap_free(c);
}

static void wipe_stack(void)
{
    volatile unsigned char buf[65536];
    for (size_t i = 0; i < sizeof(buf); i++) {
        buf[i] = 0;
    }
}

static void alloc_garbage(void)
{
    (void) heap_alloc(64);
}

static void collect_clean(void)
{
    wipe_stack();
    heap_collect();
}

static void test_heap_collect(void)
{
    volatile unsigned char frame_wipe[4096];
    memset((void *) frame_wipe, 0, sizeof(frame_wipe));
    heap_init(NULL);
    heap_reset();
    (void) heap_alloc(16);
    heap_collect();
    EXPECT(alloced_chunks.count == 0);

    heap_init(__builtin_frame_address(0));
    heap_reset();
    heap_collect();
    EXPECT(alloced_chunks.count == 0);

    {
        void *volatile live = heap_alloc(32);
        char *volatile dangling = heap + 4096;
        alloc_garbage();
        collect_clean();
        EXPECT(alloced_chunks.count == 1);
        EXPECT(chunk_list_find(&alloced_chunks, (void *) live) == 0);
        EXPECT(dangling == heap + 4096);
        heap_free((void *) live);
    }

    heap_reset();
    {
        void **volatile root = heap_alloc(sizeof(void *));
        void *child = heap_alloc(24);
        *root = child;
        child = NULL;
        collect_clean();
        EXPECT(alloced_chunks.count == 2);
        EXPECT(chunk_list_find(&alloced_chunks, (void *) root) >= 0);
        EXPECT(chunk_list_find(&alloced_chunks, *root) >= 0);
        heap_free(*root);
        heap_free((void *) root);
    }

    heap_reset();
    {
        char *p = heap_alloc(64);
        char *volatile mid = p + 16;
        p = NULL;
        collect_clean();
        EXPECT(alloced_chunks.count == 1);
        EXPECT(alloced_chunks.chunks[0].start <= (char *) mid);
        EXPECT((char *) mid < alloced_chunks.chunks[0].start + (ptrdiff_t) alloced_chunks.chunks[0].size);
        heap_free(alloced_chunks.chunks[0].start);
    }

    (void) frame_wipe[0];
}

static void test_heap_alignment(void)
{
    heap_reset();

    void *a = heap_alloc(1);
    void *b = heap_alloc(17);
    void *c = heap_alloc(32);
    EXPECT(a != NULL && b != NULL && c != NULL);
    EXPECT(((uintptr_t) a % HEAP_ALIGN) == 0);
    EXPECT(((uintptr_t) b % HEAP_ALIGN) == 0);
    EXPECT(((uintptr_t) c % HEAP_ALIGN) == 0);
    EXPECT(alloced_chunks.chunks[0].size == 16);
    EXPECT(alloced_chunks.chunks[1].size == 32);
    EXPECT(alloced_chunks.chunks[2].size == 32);
    EXPECT(heap_alloc(SIZE_MAX) == NULL);

    heap_free(a);
    heap_free(b);
    heap_free(c);
}

static void test_heap_realloc(void)
{
    heap_reset();

    EXPECT(heap_realloc(NULL, 32) != NULL);
    heap_reset();

    char *p = heap_realloc(NULL, 32);
    EXPECT(p == heap);
    EXPECT(heap_realloc(p, SIZE_MAX) == NULL);
    EXPECT(alloced_chunks.count == 1);
    memcpy(p, "hello", 6);

    char *same = heap_realloc(p, 20);
    EXPECT(same == p);
    EXPECT(alloced_chunks.chunks[0].size == 32);

    char *grown = heap_realloc(p, 48);
    EXPECT(grown == p);
    EXPECT(alloced_chunks.chunks[0].size == 48);
    EXPECT(memcmp(grown, "hello", 6) == 0);

    char *shrunk = heap_realloc(grown, 16);
    EXPECT(shrunk == p);
    EXPECT(alloced_chunks.chunks[0].size == 16);
    EXPECT(freed_chunks.count >= 1);

    EXPECT(heap_realloc(shrunk, 0) == NULL);
    EXPECT(alloced_chunks.count == 0);

    void *a = heap_alloc(32);
    void *b = heap_alloc(32);
    EXPECT(a != NULL && b != NULL);
    memcpy(a, "move-me", 8);
    void *moved = heap_realloc(a, 64);
    EXPECT(moved != a);
    EXPECT(moved == (char *) b + 32);
    EXPECT(memcmp(moved, "move-me", 8) == 0);
    heap_free(b);
    heap_free(moved);

    heap_reset();
    void *almost = heap_alloc(HEAP_CAP - 16);
    void *full = heap_realloc(almost, HEAP_CAP);
    EXPECT(full == almost);
    EXPECT(freed_chunks.count == 0);
    EXPECT(heap_realloc(full, HEAP_CAP + 1) == NULL);
    EXPECT(alloced_chunks.count == 1);
    heap_free(full);
}

static void test_heap_stats(void)
{
    heap_reset();

    Heap_Stats empty = heap_stats();
    EXPECT(empty.alloced_count == 0);
    EXPECT(empty.alloced_bytes == 0);
    EXPECT(empty.freed_count == 1);
    EXPECT(empty.freed_bytes == HEAP_CAP);
    EXPECT(empty.largest_free == HEAP_CAP);

    void *a = heap_alloc(32);
    void *b = heap_alloc(64);
    Heap_Stats used = heap_stats();
    EXPECT(used.alloced_count == 2);
    EXPECT(used.alloced_bytes == 96);
    EXPECT(used.freed_bytes == HEAP_CAP - 96);
    EXPECT(used.alloced_bytes + used.freed_bytes == HEAP_CAP);
    heap_stats_print(&used);

    heap_free(a);
    heap_free(b);
}

static void test_heap_stress(void)
{
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

    Heap_Stats stats = heap_stats();
    EXPECT(stats.alloced_bytes + stats.freed_bytes == HEAP_CAP);

    for (int i = 0; i < 128; i++) {
        heap_free(ptrs[i]);
    }
    EXPECT(alloced_chunks.count == 0);
}

int main(void)
{
    heap_init(__builtin_frame_address(0));

    test_chunk_list_insert_sorted();
    test_chunk_list_find();
    test_chunk_list_remove();
    test_chunk_list_merge();
    test_chunk_list_dump();
    test_heap_alloc_zero_and_oom();
    test_heap_alloc_split_and_write();
    test_heap_free_null_and_coalesce();
    test_heap_first_fit_skips_small_hole();
    test_heap_collect();
    test_heap_alignment();
    test_heap_realloc();
    test_heap_stats();
    test_heap_stress();

    if (tests_failed) {
        fprintf(stderr, "%d/%d expectations failed\n", tests_failed, tests_run);
        return 1;
    }

    printf("%d expectations passed\n", tests_run);
    return 0;
}
