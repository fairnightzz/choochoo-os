#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__ 1

/* doubly linked list implementation */

#include <stddef.h>
#include <stdbool.h>

typedef struct LList LList;
typedef struct LListIter LListIter;

void llist_init(); // needs to be called once before any other methods
LList* llist_new(void);
void llist_destroy(LList* list);

void llist_append_front(LList* list, void* item);
void llist_append(LList* list, void* item);

void* llist_pop_front(LList* list);
void* llist_pop(LList* list);

void* llist_front(LList* list);
void* llist_back(LList* list);

bool llist_remove_item(LList* list, void* item);
size_t llist_length(LList* list);

LListIter* llist_iter(LList* list); // ensure to free through delete this after usage to prevent memory leak.
void* llist_next(LListIter* it);
bool llist_prev(LListIter* it);
void llist_delete(LListIter* it);

#endif // __LINKED_LIST_H__
