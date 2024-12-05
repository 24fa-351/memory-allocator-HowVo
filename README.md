# Memory Allocator

This project is a custom memory allocator implemented in C. It provides an efficient way to manage dynamic memory allocation and deallocation.

## Features

- **Custom malloc**: Allocate memory blocks.
- **Custom free**: Deallocate memory blocks.
- **Memory coalescing**: Merge adjacent free blocks to reduce fragmentation.
- **Memory splitting**: Split larger blocks to fit smaller allocation requests.

## Getting Started

### Building

To build the project, run the following command: make

### Running Tests

To run all the tests, use the following command: ./memtest_dev 

or use ./memtest_dev -t num to run a single test
    num = 1 to run test_allocations();
    num = 2 to run test_interspersed_frees();
    num = 3 to run test_reallocations();
