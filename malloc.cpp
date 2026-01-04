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
    u_int64_t magic {0x00000000};
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
    assert(header->magic == 0xffffffff || header->magic == 0x12345678); // check for valid magic number
    header->free = 1;
    header->magic = 0x87654321; // mark as freed block
    merge_with_next(header);
    // TODO:: Return memory to OS if last block is free
}

void* find_free_block(size_t p_incr)
{
    //TODO:: Optimize by maintaining free list
    struct block_meta* curr = global_base;
    while(curr)
    {
        if(curr->free && curr->size >= p_incr)
        {
            curr->free = 0;
            curr->magic = 0x12345678; // reused block
            // TODO:: Split block if significantly larger than requested size
            return (void*)(curr + 1);// + 1 works because curr is of type block_meta*
        }
        curr = curr->next;
    }
    return nullptr;
}

void* extend_heap(size_t p_incr)
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
    new_block->magic = 0xffffffff; // new block
    if(!global_base)
    {
        global_base = new_block;
    }
    else
    {
        struct block_meta* last = (struct block_meta*)p - 1;
        last->next = new_block;
    }
    return (void*)(new_block + 1);
}

void* malloc(size_t p_incr)
{
    //TODO:: Align p_incr to multiple of 8 or 16 for better alignment
    //TODO:: Handle zero-size allocation, no idea how it works right now in standard malloc

    // First, try to find a free block in existing heap memory
    void* p = find_free_block(p_incr);
    if(p)
    {
        return p;
    }
    return extend_heap(p_incr);
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
    assert(header->magic == 0xffffffff || header->magic == 0x12345678); // check for valid magic number

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