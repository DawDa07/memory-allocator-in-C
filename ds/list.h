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
    List_Node *tail;
    size_t len;
} List;

void list_init(List *list);

/* Insert / remove at ends. Returns 1 on success, 0 on failure / empty. */
int list_push_front(List *list, int value);
int list_push_back(List *list, int value);
int list_pop_front(List *list, int *out);
int list_peek_front(const List *list, int *out);
int list_peek_back(const List *list, int *out);

/* Index and search. get/set/insert/remove_at use 0-based indices. */
int list_get(const List *list, size_t index, int *out);
int list_set(List *list, size_t index, int value);
int list_insert_at(List *list, size_t index, int value);
int list_remove_at(List *list, size_t index, int *out);
int list_remove(List *list, int value);
List_Node *list_find(const List *list, int value);
int list_contains(const List *list, int value);

void list_reverse(List *list);
size_t list_len(const List *list);
int list_is_empty(const List *list);
void list_free(List *list);

#endif
