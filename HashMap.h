#ifndef __HASH_MAP__
#define __HASH_MAP__

#define BUCKETS 100

#define hm_init(h, key_cmp, hash_func)  init_hash_map(&h, key_cmp, hash_func)
#define hm_insert(h, key, value)        insert(&h, key, value)
#define hm_search(h, key)               search(&h, key)
#define hm_remove(h, key)               remove_key(&h, key)
#define hm_free(h)                      free_hashmap(&h);

typedef struct HashMapEntry
{
    void *key;
    void *value;
    struct HashMapEntry *next;
} HashMapEntry;

typedef struct HashMap {
    HashMapEntry *buckets[BUCKETS];
    int (*key_cmp)(void *key1, void *key2);
    unsigned int (*hash_func)(void *key);
} HashMap;

void init_hash_map(HashMap *h, int (*key_cmp)(void *, void *), unsigned int (*hash_func)(void *));
int insert(HashMap *map, void *key, void *value);
void* search(HashMap *map, void *key);
int remove_key(HashMap *map, void *key);
int compare_strings(void *key1, void *key2);
unsigned int hash_string(void *key);
void free_hashmap(HashMap *map);

#endif // __HASH_MAP__

#ifdef __HASH_MAP_IMPLEMENTATION__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void init_hash_map(HashMap *h, int (*key_cmp)(void *, void *), unsigned int (*hash_func)(void *))
{
    for (size_t i = 0; i < BUCKETS; ++i)
    {
        h->buckets[i] = NULL;
    }
    h->key_cmp = key_cmp;
    h->hash_func = hash_func;
}

int insert(HashMap *map, void *key, void *value)
{
    unsigned int index = map->hash_func(key) % BUCKETS;
    HashMapEntry *entry = map->buckets[index];
    
    while (entry)
    {
        if (map->key_cmp(entry->key, key) == 0)
        {
            entry->value = value;
            return 1;
        }
        entry = entry->next;
    }

    HashMapEntry *new_entry = (HashMapEntry *)malloc(sizeof(HashMapEntry));
    if (!new_entry)
    {
        printf("Failed to insert %s\n", (char*)value);
        return 0;
    }
    
    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
    return 1;
}

void* search(HashMap *map, void *key) {
    unsigned int index = map->hash_func(key) % BUCKETS;
    HashMapEntry *entry = map->buckets[index];

    while (entry)
    {
        if (map->key_cmp(entry->key, key) == 0)
        {  
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

int remove_key(HashMap *map, void *key)
{
    unsigned int index = map->hash_func(key) % BUCKETS;
    HashMapEntry *entry = map->buckets[index];
    HashMapEntry *prev = NULL;

    while (entry) 
    {
        if (map->key_cmp(entry->key, key) == 0)
        {  
            if (prev)
            {
                prev->next = entry->next;
            } 
            else
            {
                map->buckets[index] = entry->next;
            }
            free(entry);
            return 1;
        }
        prev = entry;
        entry = entry->next;
    }
    return 0;
}

int compare_strings(void *key1, void *key2)
{
    return strcmp((char *)key1, (char *)key2);
}

unsigned int hash_string(void *key)
{
    unsigned int hash = 5381;
    char *name = (char*)key;
    for (unsigned int i = 0; name[i] != '\0'; ++i)
    {
        hash = ((hash << 5) + hash) + name[i];
    }
    return hash % BUCKETS;
}

void free_hashmap(HashMap *map)
{
    for (int i = 0; i < BUCKETS; i++)
    {
        HashMapEntry *entry = map->buckets[i];

        while (entry)
        {
            HashMapEntry *next = entry->next;
            free(entry);  
            entry = next;
        }
    }
    //free(map);  
}
#endif // __HASH_MAP_IMPLEMENTATION__