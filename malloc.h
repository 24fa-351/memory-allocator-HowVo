#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>

typedef struct mem_chunk
{
    int size;
    char *pointer_to_start;
} mem_chunk;

typedef struct heap
{
    mem_chunk **chunks;
    int size;
    int capacity;
} heap;

void swap(mem_chunk **a, mem_chunk **b);
void heapify(int index);
void heap_insert(mem_chunk *chunk);
void *aligned_malloc(int size);
void *my_malloc(int size);
void coalesce_free_chunks();
void my_free(void *ptr);
void *my_realloc(void *ptr, int size);

#endif