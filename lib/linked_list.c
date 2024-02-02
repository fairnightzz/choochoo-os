#include "linked_list.h"
#include <stdlib.h>
#include "alloc.h"

typedef struct LListNode {
    void* data;
    struct LListNode* next;
    struct LListNode* prev;
} LListNode;

struct LList {
    LListNode* head;
    LListNode* tail;
    size_t length;
};

struct LListIter {
    LListNode* current;
};

// Function to create a new list node
static LListNode* llist_create_node(void* item) {
    LListNode* node = alloc(LINKED_LIST_NODE);
    if (node) {
        node->data = item;
        node->next = NULL;
        node->prev = NULL;
    }
    return node;
}

void llist_init() {
  alloc_init(LINKED_LIST, sizeof(LList));
  alloc_init(LINKED_LIST_NODE, sizeof(LListNode));
  alloc_init(LINKED_LIST_ITERATOR, sizeof(LListIter));
}

LList* llist_new(void) {
    LList* list = (LList*)alloc(LINKED_LIST);
    if (list) {
        list->head = NULL;
        list->tail = NULL;
        list->length = 0;
    }
    return list;
}

void llist_destroy(LList* list) {
    LListNode* current = list->head;
    while (current) {
        LListNode* next = current->next;
        free(current, LINKED_LIST_NODE);
        current = next;
    }
    free(list, LINKED_LIST);
}

void llist_append_front(LList* list, void* item) {
    LListNode* node = llist_create_node(item);
    if (!list->head) {
        list->head = node;
        list->tail = node;
    } else {
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->length++;
}

void llist_append(LList* list, void* item) {
    LListNode* node = llist_create_node(item);
    if (!list->tail) {
        list->head = node;
        list->tail = node;
    } else {
        node->prev = list->tail;
        list->tail->next = node;
        list->tail = node;
    }
    list->length++;
}

void* llist_pop_front(LList* list) {
    if (!list->head) return NULL;
    LListNode* node = list->head;
    void* item = node->data;
    list->head = node->next;
    if (list->head) {
        list->head->prev = NULL;
    } else {
        list->tail = NULL; // List became empty
    }
    free(node, LINKED_LIST_NODE);
    list->length--;
    return item;
}

void* llist_pop(LList* list) {
    if (!list->tail) return NULL;
    LListNode* node = list->tail;
    void* item = node->data;
    list->tail = node->prev;
    if (list->tail) {
        list->tail->next = NULL;
    } else {
        list->head = NULL; // List became empty
    }
    free(node, LINKED_LIST_NODE);
    list->length--;
    return item;
}

void* llist_front(LList* list) {
    if (!list->head) return NULL;
    return list->head->data;
}

void* llist_back(LList* list) {
    if (!list->tail) return NULL;
    return list->tail->data;
}

bool llist_remove_item(LList* list, void* item) {
    for (LListNode* node = list->head; node; node = node->next) {
        if (node->data == item) {
            if (node->prev) {
                node->prev->next = node->next;
            } else {
                list->head = node->next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            } else {
                list->tail = node->prev;
            }
            free(node, LINKED_LIST_NODE);
            list->length--;
            return true;
        }
    }
    return false;
}

size_t llist_length(LList* list) {
    return list->length;
}

// Initialize an iterator starting at the head of the list
LListIter* llist_iter(LList* list) {
    LListIter* it = (LListIter*)alloc(LINKED_LIST_ITERATOR);
    if (it) {
        it->current = list->head;
    }
    return it;
}

// Get the next item in the list and advance the iterator
void* llist_next(LListIter* it) {
    if (!it->current) return NULL;
    void* data = it->current->data;
    it->current = it->current->next;
    return data;
}

// Go to the previous item in the list (if possible)
bool llist_prev(LListIter* it) {
    if (!it->current || !it->current->prev) return false;
    it->current = it->current->prev;
    return true;
}

// Delete the iterator
void llist_delete(LListIter* it) {
    free(it, LINKED_LIST_ITERATOR);
}
