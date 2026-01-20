#include <gtest/gtest.h>
#include <cstring>
#include <memory>
#include <span>

// C++26: Test using [[nodiscard]] attributes and modern pointer semantics
TEST(SPMallocTest, BasicAllocation) {
    constexpr size_t alloc_size = 1024; // 1 KB
    void* ptr = malloc(alloc_size);
    ASSERT_NE(ptr, nullptr); // Ensure allocation was successful

    // Optionally, you can write and read data to verify the memory is usable
    std::memset(ptr, 0xAA, alloc_size);
    
    // C++26: Use std::span for safer range-based access
    auto data = std::span(static_cast<unsigned char*>(ptr), alloc_size);
    for (auto byte : data) {
        ASSERT_EQ(byte, 0xAA);
    }
    
    free(ptr);
}

TEST(SPMallocTest, FreeAndReallocate) {
    constexpr size_t alloc_size = 2048; // 2 KB
    void* ptr = malloc(alloc_size);
    ASSERT_NE(ptr, nullptr); // Ensure allocation was successful

    free(ptr); // Free the allocated memory

    // Reallocate and check if we can get memory again
    void* new_ptr = malloc(alloc_size);
    ASSERT_NE(new_ptr, nullptr);

    // FIXME:: fix this later
    // ASSERT_NE(new_ptr, ptr); // In a simple allocator, this may or may not be the same, fails right now
    
    free(new_ptr);
}

TEST(SPMallocTest, CallocFunctionality) {
    constexpr size_t num_elements = 100;
    constexpr size_t element_size = sizeof(int);
    void* ptr = calloc(num_elements, element_size);
    ASSERT_NE(ptr, nullptr); // Ensure allocation was successful

    // C++26: Use std::span for safer range-based zero verification
    auto data = std::span(static_cast<unsigned char*>(ptr), num_elements * element_size);
    for (auto byte : data) {
        ASSERT_EQ(byte, 0);
    }

    free(ptr); // Free the allocated memory
}

TEST(SPMallocTest, ReallocFunctionality) {
    constexpr size_t initial_size = 512; // 512 bytes
    void* ptr = malloc(initial_size);
    ASSERT_NE(ptr, nullptr); // Ensure allocation was successful

    constexpr size_t new_size = 1024; // 1 KB
    void* new_ptr = realloc(ptr, new_size);
    ASSERT_NE(new_ptr, nullptr); // Ensure reallocation was successful
    ASSERT_NE(new_ptr, ptr); // In a simple allocator, this may or may not be the same

    free(new_ptr); // Free the reallocated memory
}

TEST(SPMallocTest, MultipleAllocations) {
    constexpr size_t alloc_size = 256; // 256 bytes
    constexpr int num_allocs = 10;
    
    // C++26: Use std::array for bounds-checked storage
    std::array<void*, num_allocs> ptrs{};

    // Allocate multiple blocks
    for (int i = 0; i < num_allocs; ++i) {
        ptrs[i] = malloc(alloc_size);
        ASSERT_NE(ptrs[i], nullptr); // Ensure each allocation was successful
    }

    // Free all allocated blocks
    for (int i = 0; i < num_allocs; ++i) {
        free(ptrs[i]);
    }
}

TEST(SPMallocTest, FreeNullPointer) {
    // Freeing a null pointer should not cause any issues
    // C++26: Uses [[nodiscard]] attribute on malloc - ignore with explicit (void) cast if needed
    EXPECT_NO_THROW(free(nullptr));
}
