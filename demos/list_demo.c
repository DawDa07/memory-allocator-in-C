#include <stdio.h>

#include "heap.h"
#include "list.h"

static void print_list(const List *list)
{
    printf("[");
    for (List_Node *n = list->head; n != NULL; n = n->next) {
        printf("%d", n->value);
        if (n->next != NULL) {
            printf(" -> ");
        }
    }
    printf("] (len=%zu)\n", list_len(list));
}

int main(void)
{
    heap_init(__builtin_frame_address(0));

    List list;
    list_init(&list);

    printf("push_back 1..5:\n  ");
    for (int i = 1; i <= 5; i++) {
        list_push_back(&list, i);
    }
    print_list(&list);

    printf("push_front 0:\n  ");
    list_push_front(&list, 0);
    print_list(&list);

    printf("insert_at(3, 99):\n  ");
    list_insert_at(&list, 3, 99);
    print_list(&list);

    printf("reverse:\n  ");
    list_reverse(&list);
    print_list(&list);

    int out = 0;
    list_pop_front(&list, &out);
    printf("pop_front -> %d:\n  ", out);
    print_list(&list);

    list_remove(&list, 99);
    printf("remove 99:\n  ");
    print_list(&list);

    list_free(&list);
    {
        Heap_Stats st = heap_stats();
        printf("after list_free: allocated chunks=%zu\n", st.alloced_count);
    }
    return 0;
}
