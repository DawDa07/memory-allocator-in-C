#include "list.h"

#include "../heap.h"

void list_init(List *list)
{
    list->head = NULL;
    list->len = 0;
}

int list_push_front(List *list, int value)
{
    List_Node *node = heap_alloc(sizeof(List_Node));
    if (node == NULL) {
        return 0;
    }
    node->value = value;
    node->next = list->head;
    list->head = node;
    list->len++;
    return 1;
}

int list_pop_front(List *list, int *out)
{
    if (list->head == NULL) {
        return 0;
    }
    List_Node *node = list->head;
    list->head = node->next;
    list->len--;
    if (out != NULL) {
        *out = node->value;
    }
    heap_free(node);
    return 1;
}

List_Node *list_find(const List *list, int value)
{
    for (List_Node *n = list->head; n != NULL; n = n->next) {
        if (n->value == value) {
            return n;
        }
    }
    return NULL;
}

int list_remove(List *list, int value)
{
    List_Node *prev = NULL;
    for (List_Node *n = list->head; n != NULL; prev = n, n = n->next) {
        if (n->value != value) {
            continue;
        }
        if (prev == NULL) {
            list->head = n->next;
        } else {
            prev->next = n->next;
        }
        list->len--;
        heap_free(n);
        return 1;
    }
    return 0;
}

size_t list_len(const List *list)
{
    return list->len;
}

void list_free(List *list)
{
    while (list->head != NULL) {
        List_Node *next = list->head->next;
        heap_free(list->head);
        list->head = next;
    }
    list->len = 0;
}
