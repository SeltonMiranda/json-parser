#ifndef __VECTOR__
#define __VECTOR__

#include <stdio.h>

#define vector_init(element, capacity)   create_dyn_array(sizeof(element), capacity)
#define vector_pop(array, item)          pop(array, item)
#define vector_reserve(array, new_cap)   reserve(array, new_cap)
#define vector_resize(array, new_size)   resize(array, new_size)
#define vector_get_size(array)           get_size(array)
#define vector_get_capacity(array)       get_capacity(array)
#define vector_get_at(array, index)      get_element_at(array, index)
#define vector_remove_at(array, index)   remove_at(array, index)
#define vector_copy(array)               copy_dyn_array(array)
#define vector_push_back(array, element) push_back(array, element)
#define vector_destroy(array)            destroy_dyn_array(array)

typedef struct Vector
{
    void *items;         
    size_t element_size; 
    size_t size;         
    size_t capacity;     
} Vector;

Vector create_dyn_array(size_t element_size, size_t capacity);
void destroy_dyn_array(Vector *array);
void reserve(Vector *array, size_t new_capacity);
void resize(Vector *array, size_t new_size);
void push_back(Vector *array, void *element);
Vector copy_dyn_array(const Vector *src);
size_t get_size(const Vector *array);
size_t get_capacity(const Vector *array);
void pop(Vector *array, void *item);
void *get_element_at(const Vector *array, size_t index);
void remove_at(Vector *array, size_t index);

#endif // __VECTOR__

#ifdef __VECTOR_IMPLEMENTATION__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Vector create_dyn_array(size_t element_size, size_t capacity) 
{
    Vector array;
    array.element_size = element_size;
    array.size = 0;
    array.capacity = capacity;
    if (capacity > 0)
    {
        array.items = (void*)malloc(capacity * element_size);
        if (!array.items)
        {
            printf("ERROR! Couldn't malloc at file %s at line %s\n", __FILE__, __LINE__);
            exit(1);
        } 
    } 
    else 
    {
        array.items = NULL;
    }
    return array;
}

void destroy_dyn_array(Vector *array) 
{
    if (array->items)
    {
        free(array->items);
    }
    array->items = NULL;
    array->size = 0;
    array->capacity = 0;
}

void reserve(Vector *array, size_t new_capacity) 
{
    if (new_capacity <= array->capacity)
    {
        return;
    }
    else
    {
        array->items = realloc(array->items, new_capacity * array->element_size);
        if (!array->items)
        {
            printf("ERROR! Couldn't malloc at file %s at line %s\n", __FILE__, __LINE__);
            exit(1);
        }
        array->capacity = new_capacity;
    }
}

void resize(Vector *array, size_t new_size) 
{
    if (new_size > array->capacity) {
        reserve(array, new_size);
    }
    memset((char *)array->items + (array->size * array->element_size), 0, (new_size - array->size) * array->element_size);
    array->size = new_size;
}

void push_back(Vector *array, void *element) 
{
    if (array->size >= array->capacity)
    {
        if (array->capacity == 0)
        {
            reserve(array, 1);
        }
        else
        {
            reserve(array, array->capacity * 2);
        }
    }
    memcpy((char *)array->items + (array->size * array->element_size), element, array->element_size);
    array->size++;
}

Vector copy_dyn_array(const Vector *src) 
{
    Vector copy = create_dyn_array(src->element_size, src->capacity);
    memcpy(copy.items, src->items, src->size * src->element_size);
    copy.size = src->size;
    return copy;
}

size_t get_size(const Vector *array) 
{
    return array->size;
}

size_t get_capacity(const Vector *array)
{
    return array->capacity;
}

void *get_element_at(const Vector *array, size_t index)
{
    if (index >= array->size) 
    {
        printf("Out of bounds, file %s, line %s\n", __FILE__, __LINE__);
        exit(1);
    }
    else
    {
        return (void *)array->items + (index * array->element_size);
    }
}

void pop(Vector *array, void *item) 
{
    if (array->size == 0)
    {
        printf("Array is empty\n");
        exit(1);
    }

    if (item) {
        memcpy(item, (char *)array->items + (array->size - 1) * array->element_size, array->element_size);
    }
    array->size--;
}

void remove_at(Vector *array, size_t index) 
{
    if (index >= array->size)
    {
        printf("Out of bounds\n");
        exit(1);
    }
    
    char *base = (char *)array->items;
    memmove(
            base + index * array->element_size, 
            base + (index + 1) * array->element_size, 
            (array->size - index - 1) * array->element_size
           );
    array->size--;
}
#endif // __DYN_ARRAY_IMPLEMENTATION__
