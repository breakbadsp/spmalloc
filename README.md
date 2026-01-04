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
