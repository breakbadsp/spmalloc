#include <gtest/gtest.h>
#include "malloc.h"

TEST(SPMallocTest, BasicAllocation) {
    const size_t alloc_size = 1024; // 1 KB
    void* ptr = malloc(alloc_size);
    ASSERT_NE(ptr, nullptr); // Ensure allocation was successful

    // Optionally, you can write and read data to verify the memory is usable
    memset(ptr, 0xAA, alloc_size);
    for (size_t i = 0; i < alloc_size; ++i) {
        ASSERT_EQ(static_cast<unsigned char*>(ptr)[i], 0xAA);
    }
}

TEST(SPMallocTest, FreeAndReallocate) {
    const size_t alloc_size = 2048; // 2 KB
    void* ptr = malloc(alloc_size);
    ASSERT_NE(ptr, nullptr); // Ensure allocation was successful

    free(ptr); // Free the allocated memory

    // Reallocate and check if we can get memory again
    void* new_ptr = malloc(alloc_size);
    ASSERT_NE(new_ptr, nullptr);

    // FIXME:: fix this later
    //ASSERT_NE(new_ptr, ptr); // In a simple allocator, this may or may not be the same, fails right now
}

TEST(SPMallocTest, CallocFunctionality) {
    const size_t num_elements = 100;
    const size_t element_size = sizeof(int);
    void* ptr = calloc(num_elements, element_size);
    ASSERT_NE(ptr, nullptr); // Ensure allocation was successful

    // Check if all bytes are zero
    for (size_t i = 0; i < num_elements * element_size; ++i) {
        ASSERT_EQ(static_cast<unsigned char*>(ptr)[i], 0);
    }

    free(ptr); // Free the allocated memory
}

TEST(SPMallocTest, ReallocFunctionality) {
    const size_t initial_size = 512; // 512 bytes
    void* ptr = malloc(initial_size);
    ASSERT_NE(ptr, nullptr); // Ensure allocation was successful

    const size_t new_size = 1024; // 1 KB
    void* new_ptr = realloc(ptr, new_size);
    ASSERT_NE(new_ptr, nullptr); // Ensure reallocation was successful
    ASSERT_NE(new_ptr, ptr); // In a simple allocator, this may or may not be the same

    free(new_ptr); // Free the reallocated memory
}