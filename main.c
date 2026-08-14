#include <stdio.h>

#include "heap.h"

typedef struct Node {
    int value;
    struct Node *next;
} Node;

static Node *list_push(Node *head, int value)
{
    Node *node = heap_alloc(sizeof(Node));
    if (node == NULL) {
        return head;
    }
    node->value = value;
    node->next = head;
    return node;
}

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

static void print_list(const char *label, Node *head)
{
    printf("%s", label);
    for (Node *n = head; n != NULL; n = n->next) {
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

    Node *head = NULL;
    for (int i = 20; i >= 1; i--) {
        head = list_push(head, i);
    }

    printf("Built list 1..20 on the custom heap.\n");
    print_list("before collect: ", head);
    {
        Heap_Stats before = heap_stats();
        heap_stats_print(&before);
        printf("allocated nodes: %zu\n", before.alloced_count);
    }

    Node *cut = head;
    for (int i = 0; i < 4 && cut != NULL; i++) {
        cut = cut->next;
    }
    if (cut != NULL) {
        cut->next = NULL;
    }

    collect_clean();

    printf("\nKept nodes 1..5 reachable; 6..20 were unreachable.\n");
    print_list("after collect:  ", head);
    {
        Heap_Stats after = heap_stats();
        heap_stats_print(&after);
        printf("allocated nodes: %zu\n", after.alloced_count);
    }

    return 0;
}
