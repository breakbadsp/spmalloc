#define _DEFAULT_SOURCE
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <bit>
#include <cstring>

// C++26 Constants: Replace magic numbers with named constants
namespace AllocatorConfig
{
    constexpr size_t MIN_BLOCK_SIZE = 10;  // Minimum block size after split
    constexpr size_t ALIGNMENT = 16;       // Allocation alignment boundary
}

struct block_meta
{
    struct block_meta* next {nullptr};
    size_t size {};
    int free {true};
};

constexpr int META_SIZE = sizeof(block_meta);

struct block_meta* global_base = nullptr;

void merge_with_next(struct block_meta* block) [[gnu::hot]]
{
    while (block->next && block->next->free) [[likely]]
    {
        block->size += META_SIZE + block->next->size;
        block->next = block->next->next;
    }
}

void free(void* p)
{
    // Free on nullptr is no-op
    if (!p) [[unlikely]] return;
    
    // C++26: Use std::bit_cast for safer pointer reinterpretation
    struct block_meta* header = std::bit_cast<struct block_meta*>(
        reinterpret_cast<uintptr_t>(p) - sizeof(block_meta)
    );
    header->free = 1;
    merge_with_next(header);
    // TODO:: Return memory to OS if last block is free
}

struct block_meta* split_block(struct block_meta* block, size_t p_incr)
{
    if (!block) [[unlikely]] return nullptr;
    
    struct block_meta* needed_block = block;
    constexpr size_t threshold = AllocatorConfig::MIN_BLOCK_SIZE + META_SIZE;
    
    if (needed_block->size >= p_incr + threshold) [[unlikely]]
    {
        // C++26: Use bit_cast for safer arithmetic on pointers
        auto char_ptr = std::bit_cast<uintptr_t>(needed_block);
        auto* splitted_block = std::bit_cast<struct block_meta*>(char_ptr + META_SIZE + p_incr);

        splitted_block->size = needed_block->size - p_incr - META_SIZE;
        splitted_block->next = needed_block->next;
        splitted_block->free = 1;
        needed_block->size = p_incr;
        needed_block->next = splitted_block;
        return needed_block;
    }

    return block;
}

void* find_free_block(struct block_meta** last, size_t p_incr) [[gnu::hot]]
{
    // TODO:: Optimize by maintaining free list
    struct block_meta* curr = global_base;
    while (curr) [[likely]]
    {
        if (curr->free && curr->size >= p_incr) [[likely]]
        {
            curr = split_block(curr, p_incr);
            curr->free = 0;
            // we do not need last if we found a block, it is needed only for extending heap
            auto offset = std::bit_cast<uintptr_t>(curr) + META_SIZE;
            return std::bit_cast<void*>(offset);
        }
        *last = curr;
        curr = curr->next;
    }
    return nullptr;
}

void* extend_heap(struct block_meta* last, size_t p_incr)
{
    // No fitting block found, request more memory
    void* p = sbrk(0);
    
    // Allocate extra space for metadata
    void* ptr = sbrk(static_cast<intptr_t>(p_incr + META_SIZE));
    if (ptr == reinterpret_cast<void*>(-1)) [[unlikely]]
    {
        return NULL;
    }

    assert(ptr == p); // thread unsafe
    
    // Initialize metadata using C++26 structured initialization
    struct block_meta* new_block = std::bit_cast<struct block_meta*>(ptr);
    new_block->size = p_incr;
    new_block->next = nullptr;
    new_block->free = 0;
    
    if (!global_base) [[unlikely]]
    {
        global_base = new_block;
    }
    else if (last) [[likely]]
    {
        last->next = new_block;
    }
    
    auto offset = std::bit_cast<uintptr_t>(new_block) + META_SIZE;
    return std::bit_cast<void*>(offset);
}

[[nodiscard]] void* malloc(size_t p_incr)
{
    // TODO:: Align p_incr to multiple of 8 or 16 for better alignment
    // TODO:: Handle zero-size allocation, no idea how it works right now in standard malloc

    // First, try to find a free block in existing heap memory
    struct block_meta* last = nullptr;
    void* p = find_free_block(&last, p_incr);
    if (p) [[likely]]
    {
        return p;
    }
    // find_free_block updates last to point to the last block
    return extend_heap(last, p_incr);
}


[[nodiscard]] void* calloc(size_t num, size_t size)
{
    size_t total_size = num * size;
    void* p = malloc(total_size);
    if (p) [[likely]]
    {
        std::memset(p, 0, total_size);
    }
    return p;
}

[[nodiscard]] void* realloc(void* ptr, size_t size)
{
    if (!ptr) [[unlikely]]
    {
        return malloc(size);
    }

    auto ptr_addr = std::bit_cast<uintptr_t>(ptr);
    struct block_meta* header = std::bit_cast<struct block_meta*>(ptr_addr - sizeof(block_meta));
    
    if (header->size >= size) [[likely]]
    {
        // Current block is already large enough
        return ptr;
    }

    // Allocate new block
    void* new_ptr = malloc(size);
    if (!new_ptr) [[unlikely]]
    {
        return NULL; // Allocation failed
    }

    // Copy old data to new block
    std::memcpy(new_ptr, ptr, header->size);
    free(ptr); // Free old block
    return new_ptr;
}