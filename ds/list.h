#ifndef DS_LIST_H_
#define DS_LIST_H_

#include <stddef.h>

/* Singly linked list of ints. Nodes come from heap_alloc.
 * Prefer list_free for deterministic cleanup; GC demos may drop roots
 * and call heap_collect instead. */
typedef struct List_Node {
    int value;
    struct List_Node *next;
} List_Node;

typedef struct {
    List_Node *head;
    size_t len;
} List;

void list_init(List *list);
int list_push_front(List *list, int value);
int list_pop_front(List *list, int *out);
List_Node *list_find(const List *list, int value);
int list_remove(List *list, int value);
size_t list_len(const List *list);
void list_free(List *list);

#endif
