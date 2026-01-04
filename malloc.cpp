#define _DEFAULT_SOURCE
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

struct block_meta
{
    struct block_meta* next {nullptr};
    size_t size {};
    int free {true};
};

constexpr int META_SIZE = sizeof(block_meta);

struct block_meta* global_base = nullptr;

void merge_with_next(struct block_meta* block)
{
    while(block->next && block->next->free)
    {
        block->size += META_SIZE + block->next->size;
        block->next = block->next->next;
    }
}

void free(void* p)
{
    //Free on nullptr is no-op
    if(!p)  return;
    struct block_meta* header = (struct block_meta*)p - 1;
    header->free = 1;
    merge_with_next(header);
    // TODO:: Return memory to OS if last block is free
}

struct block_meta* split_block(struct block_meta* block, size_t p_incr)
{
    if(!block) return nullptr;
    struct block_meta* needed_block = block;
    if(needed_block->size >= p_incr + META_SIZE + 10) // minimum block size after split
    {
        auto* splitted_block = (struct block_meta*)((char*)needed_block + META_SIZE + p_incr);

        splitted_block->size = needed_block->size - p_incr - META_SIZE;
        splitted_block->next = needed_block->next;
        splitted_block->free = 1;
        needed_block->size = p_incr;
        needed_block->next = splitted_block;
        return needed_block;
    }

    return block;
}

void* find_free_block(struct block_meta* last, size_t p_incr)
{
    //TODO:: Optimize by maintaining free list
    struct block_meta* curr = global_base;
    while(curr)
    {
        if(curr->free && curr->size >= p_incr)
        {
            curr = split_block(curr, p_incr);
            curr->free = 0;
            return (void*)((char*)curr + META_SIZE);
        }
        last = curr;
        curr = curr->next;
    }
    return nullptr;
}

void* extend_heap(struct block_meta* last, size_t p_incr)
{
    // No fitting block found, request more memory
    void* p = sbrk((intptr_t)0);
    
    // Allocate extra space for metadata
    void* ptr = sbrk((intptr_t)(p_incr + META_SIZE));
    if(ptr == (void*)-1)
    {
        return NULL;
    }

    assert(ptr == p);//thread unsafe
    
    // Initialize metadata
    struct block_meta* new_block = (struct block_meta*)ptr;
    new_block->size = p_incr;
    new_block->next = nullptr;
    new_block->free = 0;
    if(!global_base)
    {
        global_base = new_block;
    }
    else
    {
        last->next = new_block;
    }
    return (void*)((char*)new_block + META_SIZE);
}

void* malloc(size_t p_incr)
{
    //TODO:: Align p_incr to multiple of 8 or 16 for better alignment
    //TODO:: Handle zero-size allocation, no idea how it works right now in standard malloc

    // First, try to find a free block in existing heap memory
    struct block_meta* last = nullptr;
    void* p = find_free_block(last, p_incr);
    if(p)
    {
        return p;
    }
    // find_free_block updates last to point to the last block
    return extend_heap(last, p_incr);
}


void* calloc(size_t num, size_t size)
{
    size_t total_size = num * size;
    void* p = malloc(total_size);
    if(p)
    {
        memset(p, 0, total_size);
    }
    return p;
}

void* realloc(void* ptr, size_t size)
{
    if(!ptr)
    {
        return malloc(size);
    }

    struct block_meta* header = (struct block_meta*)ptr - 1;
    if(header->size >= size)
    {
        // Current block is already large enough
        return ptr;
    }

    // Allocate new block
    void* new_ptr = malloc(size);
    if(!new_ptr)
    {
        return NULL; // Allocation failed
    }

    // Copy old data to new block
    memcpy(new_ptr, ptr, header->size);
    free(ptr); // Free old block
    return new_ptr;
}