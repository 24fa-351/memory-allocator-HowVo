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

void *get_me_blocks(size_t size);
void swap(mem_chunk **a, mem_chunk **b);
void heap_insert(mem_chunk *chunk);
mem_chunk *heap_remove();
void heapify(int index);
void *my_malloc(int size);
void my_free(void *ptr);
void *my_realloc(void *ptr, size_t size);

#endif