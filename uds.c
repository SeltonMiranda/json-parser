#include "uds.h"



#include <string.h>
#include <stdlib.h>

int vector_new(Vector* vec, size_t element_size, ssize_t capacity)
{
  vec->element_size = element_size;
  vec->capacity = (size_t)capacity;
  vec->size = 0;
  vec->items = NULL;

  if (capacity < 0) {
    fprintf(stderr, "ERROR! Vector capacity must not be negative!\n");
    return 0;
  } else {
    vec->items = (void *)malloc(capacity * element_size);
    if (!vec->items) {
      fprintf(stderr, "ERROR! Couldn't allocate memory for vector\n");
      return 0;
    }
  }

  return 1;
}

void vector_deallocate(Vector* vec) 
{
  if (vec->items) {
    free(vec->items);
  }
  vec->items = NULL;
  vec->size = 0;
  vec->capacity = 0;
}

void* vector_get_ref_at(Vector* vec, ssize_t index)
{
  if (index < 0 || (size_t)index >= vec->size) {
    return NULL;
  }
  return (char*)vec->items + (index * vec->element_size);
}

int vector_reserve(Vector *vec, size_t new_capacity)
{
  if (new_capacity <= vec->capacity) {
    return 1;
  } else {
    vec->items = (void*)realloc(vec->items, new_capacity * vec->element_size);
    if (!vec->items) {
      fprintf(stderr, "ERROR! Couldn't reallocate memory for vector\n");
      return 0;
    }
    vec->capacity = new_capacity;
  }
  return 1;
}

int vector_resize(Vector *vec, size_t new_size)
{
  if (new_size > vec->capacity) {
    if (!vector_reserve(vec, new_size)) {
      return 0;
    }
  } else {
    memset((char *)vec->items + (vec->size * vec->element_size), 0, (new_size - vec->size) * vec->element_size);
    vec->size = new_size;
  }
  return 1;
}

int vector_push_back(Vector *vec, void *item)
{
  if (vec->size >= vec->capacity) {
    if (vec->capacity == 0) {
      if (!vector_reserve(vec, 1)) {
        return 0;
      }
    } else {
      if (!vector_reserve(vec, vec->capacity * 2)) {
        return 0;
      }
    }
  }
  memcpy((char*)vec->items + (vec->size * vec->element_size), item, vec->element_size);
  vec->size++;
  return 1;
}

int vector_copy(Vector* src, Vector* dst)
{
  if (!vector_new(dst, src->element_size, src->capacity)) {
    fprintf(stderr, "Could not copy\n");
    return 0;
  }
  memcpy(dst->items, src->items, src->size * src->element_size);
  dst->size = src->size;
  return 1;
}

size_t vector_get_size(Vector* vec)
{
  return vec->size;
}

size_t vector_get_capacity(Vector* vec)
{
  return vec->capacity;
}

int vector_pop(Vector* vec, void* item)
{
  if (vec->size == 0) {
    fprintf(stderr, "ERROR! Vector is empty\n");
    return 0;
  }

  if (!item) {
    fprintf(stderr, "ERROR! Parameter (void *)item not found\n");
    return 0;
  } 

  memcpy(item, (char*)vec->items + (vec->size - 1) * vec->element_size, vec->element_size);
  vec->size--;
  return 1;
}

int vector_erase(Vector* vec, ssize_t index) 
{
  if (index < 0 || (size_t)index >= vec->size) {
    fprintf(stderr, "ERROR! Index out of bounds\n");
    return 0;
  }

  char* base = (char*)vec->items;
  size_t size = vec->element_size;
  memmove(base + index * size, base + (index + 1) * size, (vec->size - index - 1) * size);
  vec->size--;
  return 1;
}



#include <ctype.h>
#include <stdlib.h>

void slice_right_trim(Slice *str)
{
  while (str->length > 0 && isspace(str->data[str->length - 1])) {
    str->length--;
  }
}

void slice_left_trim(Slice *str)
{
  while (str->length > 0 && isspace(str->data[0])) {
    str->data++;
    str->length--;
  }
}

void slice_trim(Slice *str)
{
  slice_left_trim(str);
  slice_right_trim(str);
}

int slice_equals(Slice a, Slice b)
{
  return (a.length == b.length) && (strncmp(a.data, b.data, a.length) == 0);
}

int slice_to_owned(Slice src, char **dst)
{
  *dst = (char *)malloc(sizeof(char) * (src.length + 1));
  if (!(*dst)) {
    fprintf(stderr, "ERROR! Couldn't allocate memory for owned string\n");
    return 0;
  }

  strncpy((*dst), src.data, src.length);
  (*dst)[src.length] = '\0';
  return 1;
}


#include <stdlib.h>

void hashmap_new(HashMap* map, int (*key_cmp_function)(void* key1, void* key2), unsigned int(*hash_function)(void* key))
{
  for (size_t i = 0; i < BUCKETS_SIZE; i++) {
    map->buckets[i] = NULL;
  }

  map->key_cmp_function = key_cmp_function;
  map->hash_function = hash_function;
}

int hashmap_insert(HashMap* map, void* key, void* value)
{
  unsigned int index = map->hash_function(key);
  HashMapEntry *entry = map->buckets[index];

  while (entry) {
    if (map->key_cmp_function(entry->key, key) == 0) {
      entry->value = value;
      return 1;
    }
    entry = entry->next;
  }

  HashMapEntry *new_entry = (HashMapEntry *)malloc(sizeof(HashMapEntry));
  if (!new_entry) {
    fprintf(stderr, "ERROR! Couldn't allocate memory for entry\n");
    fprintf(stderr, "Failed to insert %s\n", (char*)value);
    return 0;
  }

  new_entry->key = key;
  new_entry->value = value;
  new_entry->next = map->buckets[index];
  map->buckets[index] = new_entry;
  return 1;
}

void* hashmap_search(HashMap* map, void* key)
{
  unsigned int index = map->hash_function(key) % BUCKETS_SIZE;
  HashMapEntry *entry = map->buckets[index];

  while (entry) {
    if (map->key_cmp_function(entry->key, key) == 0) {
      return entry->value;
    }
    entry = entry->next;
  }

  // Not found
  return NULL;
}

int hashmap_remove(HashMap* map, void* key)
{
  unsigned int index = map->hash_function(key) % BUCKETS_SIZE;
  HashMapEntry *entry = map->buckets[index];
  HashMapEntry *prev = NULL;

  while (entry) {
    if (map->key_cmp_function(entry->key, key) == 0) {
      if (prev) {
        prev->next = entry->next;
      } else {
        map->buckets[index] = entry->next;
      }
      free(entry);
      return 1;
    }
    prev = entry;
    entry = entry->next;
  }
  return 0; // Not found
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
    return hash % BUCKETS_SIZE;
}

void hashmap_deallocate(HashMap *map)
{
    for (int i = 0; i < BUCKETS_SIZE; i++) {
        HashMapEntry *entry = map->buckets[i];
        while (entry) {
            HashMapEntry *next = entry->next;
            free(entry);  
            entry = next;
        }
    }
}