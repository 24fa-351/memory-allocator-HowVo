#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdalign.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include "malloc.h"

#define BLOCK_SIZE 4096

mem_chunk *free_list_chunks[100];
heap free_list = {free_list_chunks, 0, 100};
pthread_mutex_t mutex;

void swap(mem_chunk **a, mem_chunk **b)
{
    mem_chunk *temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(int index)
{
    pthread_mutex_lock(&mutex);
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    int smallest = index;

    if (left < free_list.size && free_list.chunks[left]->size < free_list.chunks[smallest]->size)
    {
        smallest = left;
    }

    if (right < free_list.size && free_list.chunks[right]->size < free_list.chunks[smallest]->size)
    {
        smallest = right;
    }

    if (smallest != index)
    {
        swap(&free_list.chunks[index], &free_list.chunks[smallest]);
        heapify(smallest);
    }
    pthread_mutex_unlock(&mutex);
}

void heap_insert(mem_chunk *chunk)
{
    pthread_mutex_lock(&mutex);
    free_list.chunks[free_list.size] = chunk;
    int current = free_list.size;
    int parent = (current - 1) / 2;

    while (current > 0 && free_list.chunks[current]->size < free_list.chunks[parent]->size)
    {
        swap(&free_list.chunks[current], &free_list.chunks[parent]);
        current = parent;
        parent = (current - 1) / 2;
    }

    free_list.size++;
    pthread_mutex_unlock(&mutex);
}

void *aligned_malloc(int size)
{
    int alignment = alignof(mem_chunk);
    void *ptr = aligned_alloc(alignment, size);
    if (ptr == NULL)
    {
        return NULL;
    }
    return ptr;
}

void *my_malloc(int size)
{
    pthread_mutex_lock(&mutex);
    if (size <= 0)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    int total_size = size + sizeof(mem_chunk);
    mem_chunk *chunk = NULL;

    for (int i = 0; i < free_list.size; i++)
    {
        if (free_list.chunks[i]->size >= total_size)
        {
            chunk = free_list.chunks[i];
            free_list.chunks[i] = free_list.chunks[free_list.size - 1];
            free_list.size--;
            break;
        }
    }

    if (chunk == NULL)
    {
        chunk = (mem_chunk *)sbrk(BLOCK_SIZE);
        if (chunk == (void *)-1)
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        chunk->size = BLOCK_SIZE;
    }

    if (chunk->size - total_size >= sizeof(mem_chunk))
    {
        mem_chunk *new_chunk = (mem_chunk *)((char *)chunk + total_size);
        new_chunk->size = chunk->size - total_size;
        heap_insert(new_chunk);
    }

    chunk->size = size;
    pthread_mutex_unlock(&mutex);
    return (void *)((char *)chunk + sizeof(mem_chunk));
}

void coalesce_free_chunks()
{
    for (int i = 0; i < free_list.size - 1; i++)
    {
        mem_chunk *current = free_list.chunks[i];
        mem_chunk *next = free_list.chunks[i + 1];

        if ((char *)current + current->size + sizeof(mem_chunk) == (char *)next)
        {
            current->size += next->size + sizeof(mem_chunk);
            for (int j = i + 1; j < free_list.size - 1; j++)
            {
                free_list.chunks[j] = free_list.chunks[j + 1];
            }
            free_list.size--;
            i--;
        }
    }
}

void my_free(void *ptr)
{
    pthread_mutex_lock(&mutex);
    if (ptr == NULL)
    {
        return;
    }

    mem_chunk *chunk = (mem_chunk *)((char *)ptr - sizeof(mem_chunk));
    heap_insert(chunk);

    coalesce_free_chunks();

    pthread_mutex_unlock(&mutex);
}

void *my_realloc(void *ptr, int new_size)
{
    if (new_size <= 0)
    {
        my_free(ptr);
        return NULL;
    }

    if (ptr == NULL)
    {
        return my_malloc(new_size);
    }

    pthread_mutex_lock(&mutex);

    mem_chunk *old_chunk = (mem_chunk *)((char *)ptr - sizeof(mem_chunk));
    int old_size = old_chunk->size;

    if (new_size <= old_size)
    {
        old_chunk->size = new_size;
        pthread_mutex_unlock(&mutex);
        return ptr;
    }

    void *new_ptr = my_malloc(new_size);
    if (new_ptr == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    int copy_size = old_size < new_size ? old_size : new_size;
    memmove(new_ptr, ptr, copy_size);

    my_free(ptr);

    pthread_mutex_unlock(&mutex);
    return new_ptr;
}