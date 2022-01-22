#pragma once
#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"

// Underlying datatype of dynamic array
typedef LinkedList data_t;

typedef struct {
    data_t *data;
    size_t size;  // size apparent to user
    size_t cap;  // true size of array
} DArray;

size_t DArray_size(const DArray *arr) {
    return arr->size;
}

void DArray_init(DArray *arr) {
    arr->data = (data_t *) malloc(sizeof(data_t));
    // Initialize data
    LL_init(arr->data);
    // Check if allocation failed
    assert(arr->data != NULL);
    
    arr->size = 0;
    arr->cap = 1;    
}

void DArray_init_reserve(DArray *arr, size_t n) {
    arr->data = (data_t *) malloc(n * sizeof(data_t));
    // Check if allocation failed
    assert(arr->data != NULL);
    // Initialize data
    for (size_t i = 0; i < n; ++i) {
        LL_init(&arr->data[i]);
    }
    
    arr->size = n;
    arr->cap = n;    
}

void DArray_insert(DArray *arr) {
    if (arr->size == arr->cap) {
        arr->cap <<= 1;  // grow capacity by factor of 2
        arr->data = (data_t *) realloc(arr->data, arr->cap * sizeof(data_t));
        // Check if reallocation failed
        assert(arr->data != NULL);
    }
    // Initialize new element & increment size
    LL_init(&arr->data[arr->size++]);
}

data_t *DArray_get(const DArray *arr, size_t i) {
    assert(i < arr->size);
    
    return &arr->data[i];
}

void DArray_free(DArray *arr) {
    // Free all linked lists
    for (size_t i = 0; i < arr->size; ++i) {
        LL_free(&arr->data[i]);
    }
    free(arr->data);
}


#endif /* DYN_ARRAY_H */
