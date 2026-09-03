#include <stdio.h>

#include "../heap.h"
#include "../ds/list.h"

static void wipe_stack(void)
{
    volatile unsigned char buf[65536];
    for (size_t i = 0; i < sizeof(buf); i++) {
        buf[i] = 0;
    }
}

static void collect_clean(void)
{
    wipe_stack();
    heap_collect();
}

static void print_list(const char *label, List_Node *head)
{
    printf("%s", label);
    for (List_Node *n = head; n != NULL; n = n->next) {
        printf("%d", n->value);
        if (n->next != NULL) {
            printf(" -> ");
        }
    }
    printf("\n");
}

int main(void)
{
    heap_init(__builtin_frame_address(0));

    List list;
    list_init(&list);
    for (int i = 20; i >= 1; i--) {
        if (!list_push_front(&list, i)) {
            fprintf(stderr, "list_push_front failed\n");
            return 1;
        }
    }

    printf("Built list 1..20 on the custom heap via ds/list.\n");
    print_list("before collect: ", list.head);
    {
        Heap_Stats before = heap_stats();
        heap_stats_print(&before);
        printf("allocated nodes: %zu\n", before.alloced_count);
    }

    /* Cut after node 5 so 6..20 become unreachable (GC demo, not list_free). */
    List_Node *cut = list.head;
    for (int i = 0; i < 4 && cut != NULL; i++) {
        cut = cut->next;
    }
    if (cut != NULL) {
        cut->next = NULL;
    }
    list.len = 5;

    collect_clean();

    printf("\nKept nodes 1..5 reachable; 6..20 were unreachable.\n");
    print_list("after collect:  ", list.head);
    {
        Heap_Stats after = heap_stats();
        heap_stats_print(&after);
        printf("allocated nodes: %zu\n", after.alloced_count);
    }

    list_free(&list);
    return 0;
}
