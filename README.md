# spmalloc

Purpose
-------

This repository is an educational implementation of a simple memory allocator. The primary goal is to learn and experiment with how `malloc`/`free`/`realloc`/`calloc` can be implemented and to explore allocator internals (metadata layout, splitting/coalescing, sbrk/brk usage, etc.).

Not production-ready
--------------------

This code is experimental and intentionally simple. It is not production ready at any level — it lacks thread-safety, rigorous error handling, security hardening, thorough testing, and portability guarantees. Do not use this implementation in production or in untrusted environments.

Reference
---------

- Primary reference: [`danluu malloc tutorial`](https://danluu.com/malloc-tutorial/).  


Testing
-------

- Unit tests are provided under `gtest/` but test coverage is still in progress.  
- Testing with sanitizers (AddressSanitizer, UndefinedBehaviorSanitizer, etc.) is planned and pending — these tools are recommended when developing and validating allocators.

Building & running tests (quick)
--------------------------------

- Build the shared object (example):

```bash
make
```

- Configure and run the GoogleTest-based tests (from `gtest/`):

```bash
cd gtest
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --verbose
```

Notes
-----

- The allocator may be built and linked into tests directly, or used as a `.so` and preloaded via `LD_PRELOAD` for integration-style runs. See `gtest/CMakeLists.txt` for test wiring.
- If you modify metadata layout or alignment, update unit tests accordingly.

Contributions
-------------

Patches, experiments, and test improvements are welcome — this repo is intended as a learning playground.

License
-------

This repository does not include a license file. Treat it as experimental code. Add a license if you intend to distribute it.

Open Issues
-------

| Priority | Category       | TODO / Extension Description                                                                 | Why It's Useful / Common in Real Allocators                                                                 | Difficulty / Notes                                                                 |
|----------|----------------|----------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------|
| 1        | Core Correctness | **Add proper alignment** (round requests and payload pointers to 16 bytes)                 | Ensures compliance with C standards; prevents crashes with aligned data types. Mentioned as a key check in malloc tutorials. | Low: In `malloc`, add `p_incr = (p_incr + 15) & ~15ULL;`. Verify META_SIZE alignment. |
| 2        | Core Correctness | **Handle malloc(0) and calloc overflows**                                           | Standards-compliant handling of zero-size; prevent UB from overflow in calloc. | Low: Return NULL for size==0; add overflow check in calloc. |
| 3        | Core Correctness | **Bidirectional coalescing** (merge with previous free block on free)                       | Reduces fragmentation by merging both directions. Essential for efficiency, as noted in multiple implementations. | Medium: Add `prev` pointer to `block_meta` for doubly-linked list; merge prev/next on free. |
| 4        | Performance    | **Maintain explicit free list** (separate linked list of only free blocks)                   | Speeds up allocation from O(n) to near O(1). Common optimization in custom allocators. | Medium: Add free_list global; insert on free, search/remove in alloc. |
| 5        | User Suggestion | **Use mmap for large allocations** (e.g., > 128KB threshold) instead of sbrk                | Allows returning large blocks to OS via munmap; reduces fragmentation, better for huge requests. Standard in glibc/jemalloc. | Medium: In malloc, if p_incr > THRESHOLD, use mmap; track separately for free (munmap). Include <sys/mman.h>. |
| 6        | User Suggestion | **Make it thread-safe** (add mutex around all operations)                                   | Prevents races in multithreaded use. Critical for real-world apps. | Low-Medium: Add global std::mutex (include <mutex>); lock in malloc/free/realloc. Note: Code is C++-ish, so possible. |
| 7        | Efficiency     | **Optimize realloc** (extend/shrink in place if possible, without always copy+free)         | Avoids unnecessary copies; splits on shrink, merges/extends if next free. Improves performance. | Medium: In realloc, if size > current and next free, merge+split; if smaller, split. |
| 8        | Efficiency     | **Preallocate extra space on heap extension** (request more than needed via sbrk)            | Reduces frequent sbrk calls, which are expensive. Common to allocate pages (e.g., 4KB+). | Low: In extend_heap, request max(p_incr, PAGE_SIZE) or similar. |
| 9        | Efficiency     | **Change to best-fit or first-fit with improvements** (instead of current first-fit)         | Best-fit minimizes waste/fragmentation over first-fit. | Medium: In find_free_block, scan for smallest fitting block. Needs free list for efficiency. |
| 10       | Robustness     | **Return memory to OS** (shrink heap with sbrk negative or munmap)                           | Frees unused memory, reduces process size. Especially for end-of-heap free blocks. | Medium: After free/merge, if last block free, sbrk(-(size + META_SIZE)). |
| 11       | Robustness     | **Add debugging aids** (magic numbers, canaries, heap validation)                            | Detects corruption, double-free, overflows early. Useful for development. | Low: Add magic field; set on alloc/free; assert in ops. Add guard bytes at block end. |
| 12       | Advanced       | **Segregated free lists / size classes** (buckets for different sizes)                       | Faster small allocations; reduces fragmentation. Core to jemalloc/tcmalloc. | High: Array of free lists for size bins (e.g., powers of 2). |
| 13       | Advanced       | **Per-thread arenas/caches** (for thread-local allocations)                                  | Minimizes lock contention in multithreaded; boosts scalability. | High: Use thread-local storage; fall back to global locked heap. |
| 14       | Testing        | **Add automated tests** (unit tests for malloc/free/realloc edge cases)                      | Ensures correctness; catches bugs like fragmentation, leaks. Recommended in tutorials. | Medium: Write test harness with allocations, frees, checks (e.g., valgrind-compatible). |
| 15       | Standards      | **Support aligned_alloc / memalign** (for higher alignments like 32/64 bytes)                | Needed for SIMD/AVX; modern C11 feature. | Medium: Allocate extra, adjust pointer to alignment, store original in meta. |