#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

typedef struct __attribute__((aligned(8))) mem_chunk
{
    size_t size;
    void *pointer_to_start;
} mem_chunk;

typedef struct heap
{
    mem_chunk **chunks;
    size_t size;
    size_t capacity;
} heap;

#define BLOCK_SIZE 4096

mem_chunk *free_list_chunks[50];
heap free_list = {(mem_chunk **)&free_list_chunks, 0, 50};
pthread_mutex_t mutex;

void *get_me_blocks(size_t size)
{
    void *ptr = sbrk(0);
    if (posix_memalign(&ptr, 8, BLOCK_SIZE) != 0)
    {
        return NULL;
    }
    if (sbrk(size) == (void *)-1)
    {
        return NULL;
    }
    return ptr;
}

void swap(mem_chunk **a, mem_chunk **b)
{
    mem_chunk *temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(size_t index)
{
    pthread_mutex_lock(&mutex);
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;
    size_t smallest = index;

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

mem_chunk *heap_remove()
{
    pthread_mutex_lock(&mutex);
    if (free_list.size == 0)
    {
        return NULL;
    }

    mem_chunk *chunk = free_list.chunks[0];
    free_list.size--;
    free_list.chunks[0] = free_list.chunks[free_list.size];
    heapify(0);
    pthread_mutex_unlock(&mutex);

    return chunk;
}

void *my_malloc(int size)
{
    if (size <= 0)
    {
        return NULL;
    }

    pthread_mutex_lock(&mutex);

    for (size_t i = 0; i < free_list.size; i++)
    {
        if (free_list.chunks[i]->size >= size)
        {
            mem_chunk *chunk = free_list.chunks[i];
            size_t leftover_size = chunk->size - size - sizeof(mem_chunk);

            if (leftover_size > 0)
            {
                mem_chunk *leftover = (mem_chunk *)((char *)chunk + sizeof(mem_chunk) + size);
                leftover->size = leftover_size;
                leftover->pointer_to_start = (char *)leftover + sizeof(mem_chunk);
                heap_insert(leftover);
            }

            chunk->size = size;
            free_list.chunks[i] = free_list.chunks[--free_list.size];
            heap_remove();
            return (char *)chunk + sizeof(mem_chunk);
        }
    }

    int total_size = size + sizeof(mem_chunk);
    mem_chunk *new_chunk = (mem_chunk *)get_me_blocks(BLOCK_SIZE);
    if (new_chunk == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    new_chunk->size = BLOCK_SIZE;
    new_chunk->pointer_to_start = (char *)new_chunk + sizeof(mem_chunk);

    size_t leftover_size = BLOCK_SIZE - total_size;
    if (leftover_size > 0)
    {
        mem_chunk *leftover = (mem_chunk *)((char *)new_chunk + total_size);
        leftover->size = leftover_size;
        leftover->pointer_to_start = (char *)leftover + sizeof(mem_chunk);
        heap_insert(leftover);
    }

    pthread_mutex_unlock(&mutex);
    return new_chunk->pointer_to_start;
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
    pthread_mutex_unlock(&mutex);
}

void *my_realloc(void *ptr, size_t size)
{
    pthread_mutex_lock(&mutex);
    if (ptr == NULL)
    {
        return my_malloc(size);
    }

    if (size == 0)
    {
        my_free(ptr);
        return NULL;
    }

    mem_chunk *old_chunk = (mem_chunk *)((char *)ptr - sizeof(mem_chunk));
    size_t old_size = old_chunk->size;

    void *new_ptr = my_malloc(size);
    if (new_ptr == NULL)
    {
        return NULL;
    }

    size_t copy_size = old_size < size ? old_size : size;
    memmove(new_ptr, ptr, copy_size);

    my_free(ptr);

    pthread_mutex_unlock(&mutex);
    return new_ptr;
}
