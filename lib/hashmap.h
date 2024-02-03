#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HM_BUCKETS 67
typedef struct HashMap HashMap;
typedef char *key_t;
typedef void *value_t;

void hashmap_init(); // must be called once before any of the below functions
HashMap *hashmap_new();
void hashmap_delete(HashMap *hm);
int hashmap_insert(HashMap *hm, key_t key, value_t value);
bool hashmap_contains(HashMap *hm, key_t key);
bool hashmap_remove(HashMap *hm, key_t key);
value_t hashmap_get(HashMap *hm, key_t key, bool *success);
size_t hashmap_length(HashMap *hm);

#endif // __HASHMAP_H__
