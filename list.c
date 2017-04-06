#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"
#include "graphics.h"

void list_new(list *list, int elementSize, freeFunction freeFn) {
    assert(elementSize > 0);
    list->logicalLength = 0;
    list->elementSize = elementSize;
    list->head = list->tail = NULL;
    list->freeFn = freeFn;
}

void list_destroy(list *list) {
    listNode *current;
    while (list->head != NULL) {
        current = list->head;
        list->head = current->next;

        if (list->freeFn) {
            list->freeFn(current->data);
        }

        free(current->data);
        free(current);
    }
}

void list_prepend(list *list, void *element) {
    listNode *node = malloc(sizeof(listNode));
    node->data = malloc((size_t) list->elementSize);
    memcpy(node->data, element, (size_t) list->elementSize);

    node->next = list->head;
    list->head = node;

    // first node?
    if (!list->tail) {
        list->tail = list->head;
    }

    list->logicalLength++;
}

void list_append(list *list, void *element) {
    listNode *node = malloc(sizeof(listNode));
    node->data = malloc((size_t) list->elementSize);
    node->next = NULL;

    memcpy(node->data, element, (size_t) list->elementSize);

    if (list->logicalLength == 0) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    list->logicalLength++;
}

void list_for_each(list *list, listIterator iterator) {
    assert(iterator != NULL);

    listNode *node = list->head;
    bool result = (bool) TRUE;
    while (node != NULL && result) {
        result = iterator(node->data);
        node = node->next;
    }
}

void list_head(list *list, void *element, bool removeFromList) {
    assert(list->head != NULL);

    listNode *node = list->head;
    memcpy(element, node->data, (size_t) list->elementSize);

    if (removeFromList) {
        list->head = node->next;
        list->logicalLength--;

        free(node->data);
        free(node);
    }
}

void list_delete_last(list *list) {
    listNode *temp = list->head;
    listNode *t;

    while(temp->next != NULL) {
        t = temp;
        temp = temp->next;
    }

    free(t->next->data);
    free(t->next);
    t->next = NULL;

    list->logicalLength--;
}

void list_tail(list *list, void *element) {
    assert(list->tail != NULL);
    listNode *node = list->tail;
    memcpy(element, node->data, (size_t) list->elementSize);
}

int list_size(list *list) {
    return list->logicalLength;
}

int list_count_occurences(list *list, void *data) {
    listNode *temp = list->head;

    int i = 0;

    while(temp->next != NULL) {
        if(!memcmp(temp->data, data, sizeof(POINT)))
            i++;
        temp = temp->next;
    }
    return i;
}

int list_has_duplicate(list *list) {
    listNode *temp = list->head;

    while(temp->next != NULL) {
        if(list_count_occurences(list, temp->data) > 1)
            return 1;
        temp = temp->next;
    }
    return 0;
}