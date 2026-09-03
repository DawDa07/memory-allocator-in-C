#include <stdio.h>
#include <string.h>

#include "heap.h"
#include "ds/vec.h"
#include "ds/list.h"
#include "ds/hashmap.h"

static int tests_run;
static int tests_failed;

#define EXPECT(cond) do { \
    tests_run++; \
    if (!(cond)) { \
        tests_failed++; \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

static void test_vec_grow_and_pop(void)
{
    heap_reset();
    Vec v;
    vec_init(&v);

    EXPECT(vec_len(&v) == 0);
    EXPECT(vec_pop(&v, NULL) == 0);

    for (int i = 0; i < 100; i++) {
        EXPECT(vec_push(&v, i) == 1);
    }
    EXPECT(vec_len(&v) == 100);

    int x = -1;
    EXPECT(vec_get(&v, 0, &x) == 1 && x == 0);
    EXPECT(vec_get(&v, 99, &x) == 1 && x == 99);
    EXPECT(vec_get(&v, 100, &x) == 0);
    EXPECT(vec_set(&v, 50, 500) == 1);
    EXPECT(vec_get(&v, 50, &x) == 1 && x == 500);

    EXPECT(vec_pop(&v, &x) == 1 && x == 99);
    EXPECT(vec_len(&v) == 99);

    /* One contiguous buffer → a single allocated chunk. */
    {
        Heap_Stats st = heap_stats();
        EXPECT(st.alloced_count == 1);
    }

    vec_free(&v);
    EXPECT(vec_len(&v) == 0);
    {
        Heap_Stats st = heap_stats();
        EXPECT(st.alloced_count == 0);
    }
}

static void test_vec_oom(void)
{
    heap_reset();
    Vec v;
    vec_init(&v);

    /* Exhaust the arena with one huge push attempt via reserve growth. */
    int pushed = 0;
    while (vec_push(&v, pushed)) {
        pushed++;
        if (pushed > 200000) {
            break;
        }
    }
    EXPECT(pushed > 0);
    EXPECT(vec_push(&v, 1) == 0);

    vec_free(&v);
}

static void test_list_ops(void)
{
    heap_reset();
    List list;
    list_init(&list);

    EXPECT(list_pop_front(&list, NULL) == 0);
    EXPECT(list_push_front(&list, 3) == 1);
    EXPECT(list_push_front(&list, 2) == 1);
    EXPECT(list_push_front(&list, 1) == 1);
    EXPECT(list_len(&list) == 3);
    EXPECT(list.head->value == 1);
    EXPECT(list_find(&list, 2) != NULL);
    EXPECT(list_find(&list, 99) == NULL);

    EXPECT(list_remove(&list, 2) == 1);
    EXPECT(list_len(&list) == 2);
    EXPECT(list_find(&list, 2) == NULL);
    EXPECT(list_remove(&list, 2) == 0);

    int out = 0;
    EXPECT(list_pop_front(&list, &out) == 1 && out == 1);
    EXPECT(list_pop_front(&list, &out) == 1 && out == 3);
    EXPECT(list_len(&list) == 0);

    list_free(&list);
}

static void test_hashmap_ops(void)
{
    heap_reset();
    HashMap m;
    hashmap_init(&m);

    int out = 0;
    EXPECT(hashmap_get(&m, 1, &out) == 0);
    EXPECT(hashmap_put(&m, 1, 10) == 1);
    EXPECT(hashmap_put(&m, 2, 20) == 1);
    EXPECT(hashmap_put(&m, 3, 30) == 1);
    EXPECT(hashmap_len(&m) == 3);

    EXPECT(hashmap_get(&m, 2, &out) == 1 && out == 20);
    EXPECT(hashmap_put(&m, 2, 22) == 1);
    EXPECT(hashmap_get(&m, 2, &out) == 1 && out == 22);
    EXPECT(hashmap_len(&m) == 3);

    EXPECT(hashmap_remove(&m, 2) == 1);
    EXPECT(hashmap_get(&m, 2, &out) == 0);
    EXPECT(hashmap_len(&m) == 2);
    EXPECT(hashmap_remove(&m, 2) == 0);

    /* Re-insert into tombstone slot. */
    EXPECT(hashmap_put(&m, 2, 200) == 1);
    EXPECT(hashmap_get(&m, 2, &out) == 1 && out == 200);

    /* Force several rehashes. */
    for (int i = 100; i < 200; i++) {
        EXPECT(hashmap_put(&m, i, i * 2) == 1);
    }
    EXPECT(hashmap_get(&m, 150, &out) == 1 && out == 300);
    EXPECT(hashmap_get(&m, 1, &out) == 1 && out == 10);

    hashmap_free(&m);
    EXPECT(hashmap_len(&m) == 0);
    {
        Heap_Stats st = heap_stats();
        EXPECT(st.alloced_count == 0);
    }
}

static void test_chunk_pressure_list_vs_vec(void)
{
    heap_reset();

    List list;
    list_init(&list);
    for (int i = 0; i < 64; i++) {
        EXPECT(list_push_front(&list, i) == 1);
    }
    Heap_Stats list_stats = heap_stats();
    EXPECT(list_stats.alloced_count == 64);
    list_free(&list);

    heap_reset();
    Vec v;
    vec_init(&v);
    for (int i = 0; i < 64; i++) {
        EXPECT(vec_push(&v, i) == 1);
    }
    Heap_Stats vec_stats = heap_stats();
    EXPECT(vec_stats.alloced_count == 1);
    vec_free(&v);
}

int main(void)
{
    heap_init(__builtin_frame_address(0));

    test_vec_grow_and_pop();
    test_vec_oom();
    test_list_ops();
    test_hashmap_ops();
    test_chunk_pressure_list_vs_vec();

    if (tests_failed == 0) {
        printf("ok — %d expectations passed\n", tests_run);
        return 0;
    }
    printf("%d/%d expectations failed\n", tests_failed, tests_run);
    return 1;
}
