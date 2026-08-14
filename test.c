#include <stdio.h>
#include <stddef.h>
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
    EXPECT(alloced_chunks.chunks[0].size == 26);
    EXPECT(freed_chunks.count == 1);
    EXPECT(freed_chunks.chunks[0].start == heap + 26);
    EXPECT(freed_chunks.chunks[0].size == HEAP_CAP - 26);

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

    if (tests_failed) {
        fprintf(stderr, "%d/%d expectations failed\n", tests_failed, tests_run);
        return 1;
    }

    printf("%d expectations passed\n", tests_run);
    return 0;
}
