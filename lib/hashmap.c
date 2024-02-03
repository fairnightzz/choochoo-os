#include "hashmap.h"
#include "linked_list.h"
#include <stdlib.h>
#include <string.h>
#include "alloc.h"

typedef struct HashMapNode
{
    key_t key;
    value_t value;
} HashMapNode;

struct HashMap
{
    LList **buckets;
    size_t size;
};

unsigned int hash(key_t key)
{
    // A simple hash function using prime 31
    unsigned int hash = 0;
    for (size_t i = 0; key[i] != '\0'; i++)
    {
        hash = 31 * hash + key[i];
    }
    return hash % HM_BUCKETS;
}

void hashmap_init()
{
    alloc_init(HASH_MAP, sizeof(HASH_MAP));
    alloc_init(HASH_MAP_NODE, sizeof(HASH_MAP_NODE));
    alloc_init(HASH_MAP_BUCKETS, HM_BUCKETS * sizeof(LList *));
}

HashMap *hashmap_new()
{
    HashMap *hm = (HashMap *)alloc(HASH_MAP);
    if (!hm)
        return NULL;

    hm->size = 0;
    hm->buckets = (LList **)alloc(HASH_MAP_BUCKETS);
    if (!hm->buckets)
    {
        free(hm, HASH_MAP);
        return NULL;
    }

    for (size_t i = 0; i < HM_BUCKETS; i++)
    {
        hm->buckets[i] = llist_new();
        if (!hm->buckets[i])
        {
            for (size_t j = 0; j < i; j++)
            {
                llist_destroy(hm->buckets[j]);
            }
            free(hm->buckets, HASH_MAP_BUCKETS);
            free(hm, HASH_MAP);
            return NULL;
        }
    }
    return hm;
}

void hashmap_delete(HashMap *hm)
{
    for (size_t i = 0; i < HM_BUCKETS; i++)
    {
        LList *list = hm->buckets[i];
        LListIter *it = llist_iter(list);
        HashMapNode *node;
        while ((node = (HashMapNode *)llist_next(it)) != NULL)
        {
            free(node->key, HASH_MAP_NODE); // Assuming keys are dynamically allocated
            free(node, HASH_MAP_NODE);      // Free the node itself
        }
        llist_delete(it);
        llist_destroy(list);
    }
    free(hm->buckets, HASH_MAP_BUCKETS);
    free(hm, HASH_MAP);
}

int hashmap_insert(HashMap *hm, key_t key, value_t value)
{
    unsigned int bucket_index = hash(key);
    LList *bucket = hm->buckets[bucket_index];
    LListIter *it = llist_iter(bucket);

    HashMapNode *node;
    while ((node = (HashMapNode *)llist_next(it)) != NULL)
    {
        if (strcmp(node->key, key) == 0)
        {
            // Key already exists, update the value
            node->value = value;
            llist_delete(it);
            return 0;
        }
    }
    llist_delete(it);

    // Key does not exist, create a new node and add it to the bucket
    HashMapNode *new_node = (HashMapNode *)alloc(HASH_MAP_NODE);
    if (!new_node)
        return -1; // Allocation failed

    // Assuming the key is a string that should be copied (deep copy)
    new_node->key = key;
    new_node->value = value;
    llist_append(bucket, new_node);
    hm->size++;
    return 0;
}

bool hashmap_contains(HashMap *hm, key_t key)
{
    unsigned int bucket_index = hash(key);
    LList *bucket = hm->buckets[bucket_index];
    LListIter *it = llist_iter(bucket);

    HashMapNode *node;
    while ((node = (HashMapNode *)llist_next(it)) != NULL)
    {
        if (strcmp(node->key, key) == 0)
        {
            llist_delete(it);
            return true;
        }
    }
    llist_delete(it);
    return false;
}

bool hashmap_remove(HashMap *hm, key_t key)
{
    unsigned int bucket_index = hash(key);
    LList *bucket = hm->buckets[bucket_index];
    LListIter *it = llist_iter(bucket);

    HashMapNode *node;
    while ((node = (HashMapNode *)llist_next(it)) != NULL)
    {
        if (strcmp(node->key, key) == 0)
        {
            // Found the key, remove the node from the list
            llist_remove_item(bucket, node);
            free(node, HASH_MAP_NODE); // Free the node itself
            llist_delete(it);
            hm->size--;
            return true;
        }
    }
    llist_delete(it);
    return false;
}

value_t hashmap_get(HashMap *hm, key_t key, bool *success)
{
    if (success)
    {
        *success = false; // Default to false, will set to true if key is found
    }

    unsigned int bucket_index = hash(key);
    LList *bucket = hm->buckets[bucket_index];
    LListIter *it = llist_iter(bucket);

    HashMapNode *node;
    while ((node = (HashMapNode *)llist_next(it)) != NULL)
    {
        if (strcmp(node->key, key) == 0)
        {
            // Key found
            if (success)
            {
                *success = true;
            }
            llist_delete(it);   // Delete the iterator
            return node->value; // Return the associated value
        }
    }
    llist_delete(it); // Delete the iterator
    return NULL;      // Key not found, return NULL
}

size_t hashmap_length(HashMap *hm)
{
    return hm->size;
}
