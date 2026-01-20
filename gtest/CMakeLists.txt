cmake_minimum_required(VERSION 3.14)

project(SpMallocTests LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use FetchContent to download and build GoogleTest when it's not already
# available on the system. This keeps the test config self-contained.
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


# Source (point to test source in the project root)
set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/SpMallocTests.cpp
)

# Create the test executable
add_executable(SpMallocTests ${SOURCES})



# Include project headers (parent directory) and configure warnings
target_include_directories(SpMallocTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_compile_options(SpMallocTests PRIVATE -Wall -Wextra -Wpedantic -Werror)

# C++26 Modern Sanitizers: Enable AddressSanitizer and UndefinedBehaviorSanitizer for robust testing
if(NOT MSVC)
    target_compile_options(SpMallocTests PRIVATE 
        -fsanitize=address,undefined
        -fno-sanitize-recover=all
        -fno-omit-frame-pointer
    )
    target_link_options(SpMallocTests PRIVATE
        -fsanitize=address,undefined
    )
endif()

# Link with GoogleTest
target_link_libraries(
  SpMallocTests
  GTest::gtest_main
)

# Enable testing and register tests with CTest
enable_testing()
include(GoogleTest)
gtest_discover_tests(SpMallocTests)