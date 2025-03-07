#ifndef __UDS__
#define __UDS__

#include <stdio.h>
#include <sys/types.h>

typedef struct Vector {
    void *items;         
    size_t element_size; 
    size_t size;         
    size_t capacity;     
} Vector;

int vector_new(Vector* vec, size_t element_size, ssize_t capacity);
int vector_reserve(Vector* vec, size_t new_capacity);
int vector_push_back(Vector* vec, void* item);
void vector_deallocate(Vector* vec);
int vector_copy(Vector* src, Vector* dst);
size_t vector_get_size(Vector* vec);
size_t vector_get_capacity(Vector* vec);
int vector_pop(Vector* vec, void* item);
int vector_erase(Vector* vec, ssize_t index);
int vector_resize(Vector* vec, size_t new_size);
void* vector_get_ref_at(Vector* vec, ssize_t index);

#include <string.h>
#include <stdlib.h>



#include <string.h>

#define slice_null (Slice){.data = NULL, .length = 0}
#define _slice(c_str) (Slice){.data = c_str, .length = (c_str) ? strlen(c_str) : 0} 
#define slice_args(str) (int)(str.length), (str.data)
#define slice_fmt "%.*s"

typedef struct Slice {
  char *data;
  size_t length;
} Slice;

void slice_right_trim(Slice *str);
void slice_left_trim(Slice *str);
void slice_trim(Slice *str);
int slice_equals(Slice a, Slice b);
int slice_to_owned(Slice src, char **dst);

#include <ctype.h>
#include <stdlib.h>



#define BUCKETS_SIZE 100

typedef struct HashMapEntry {
  struct HashMapEntry* next;
  void* key;
  void* value;
} HashMapEntry;

typedef struct HashMap {
  HashMapEntry* buckets[BUCKETS_SIZE];
  int (*key_cmp_function)(void* key1, void* key2);
  unsigned int (*hash_function)(void* key);
} HashMap;

void hashmap_new(HashMap* map, int (*key_cmp_function)(void* key1, void* key2), unsigned int(*hash_function)(void* key));
int hashmap_insert(HashMap* map, void* key, void* value);
void* hashmap_search(HashMap* map, void* key);
int hashmap_remove(HashMap* map, void* key);
void hashmap_deallocate(HashMap* map);
int compare_strings(void *key1, void *key2);
unsigned int hash_string(void *key);





#endif // __UDS__