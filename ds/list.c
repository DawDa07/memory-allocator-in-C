#include "list.h"

#include "heap.h"

void list_init(List *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->len = 0;
}

static List_Node *list_new_node(int value)
{
    List_Node *node = heap_alloc(sizeof(List_Node));
    if (node == NULL) {
        return NULL;
    }
    node->value = value;
    node->next = NULL;
    return node;
}

static List_Node *list_node_at(const List *list, size_t index)
{
    if (index >= list->len) {
        return NULL;
    }
    List_Node *n = list->head;
    for (size_t i = 0; i < index; i++) {
        n = n->next;
    }
    return n;
}

int list_push_front(List *list, int value)
{
    List_Node *node = list_new_node(value);
    if (node == NULL) {
        return 0;
    }
    node->next = list->head;
    list->head = node;
    if (list->tail == NULL) {
        list->tail = node;
    }
    list->len++;
    return 1;
}

int list_push_back(List *list, int value)
{
    List_Node *node = list_new_node(value);
    if (node == NULL) {
        return 0;
    }
    if (list->tail == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
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
    if (list->head == NULL) {
        list->tail = NULL;
    }
    list->len--;
    if (out != NULL) {
        *out = node->value;
    }
    heap_free(node);
    return 1;
}

int list_peek_front(const List *list, int *out)
{
    if (list->head == NULL || out == NULL) {
        return 0;
    }
    *out = list->head->value;
    return 1;
}

int list_peek_back(const List *list, int *out)
{
    if (list->tail == NULL || out == NULL) {
        return 0;
    }
    *out = list->tail->value;
    return 1;
}

int list_get(const List *list, size_t index, int *out)
{
    List_Node *n = list_node_at(list, index);
    if (n == NULL || out == NULL) {
        return 0;
    }
    *out = n->value;
    return 1;
}

int list_set(List *list, size_t index, int value)
{
    List_Node *n = list_node_at(list, index);
    if (n == NULL) {
        return 0;
    }
    n->value = value;
    return 1;
}

int list_insert_at(List *list, size_t index, int value)
{
    if (index > list->len) {
        return 0;
    }
    if (index == 0) {
        return list_push_front(list, value);
    }
    if (index == list->len) {
        return list_push_back(list, value);
    }

    List_Node *prev = list_node_at(list, index - 1);
    List_Node *node = list_new_node(value);
    if (prev == NULL || node == NULL) {
        if (node != NULL) {
            heap_free(node);
        }
        return 0;
    }
    node->next = prev->next;
    prev->next = node;
    list->len++;
    return 1;
}

int list_remove_at(List *list, size_t index, int *out)
{
    if (index >= list->len) {
        return 0;
    }
    if (index == 0) {
        return list_pop_front(list, out);
    }

    List_Node *prev = list_node_at(list, index - 1);
    List_Node *node = prev->next;
    prev->next = node->next;
    if (node == list->tail) {
        list->tail = prev;
    }
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

int list_contains(const List *list, int value)
{
    return list_find(list, value) != NULL;
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
        if (n == list->tail) {
            list->tail = prev;
        }
        list->len--;
        heap_free(n);
        return 1;
    }
    return 0;
}

void list_reverse(List *list)
{
    List_Node *prev = NULL;
    List_Node *cur = list->head;
    list->tail = list->head;
    while (cur != NULL) {
        List_Node *next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    list->head = prev;
}

size_t list_len(const List *list)
{
    return list->len;
}

int list_is_empty(const List *list)
{
    return list->len == 0;
}

void list_free(List *list)
{
    while (list->head != NULL) {
        List_Node *next = list->head->next;
        heap_free(list->head);
        list->head = next;
    }
    list->tail = NULL;
    list->len = 0;
}
