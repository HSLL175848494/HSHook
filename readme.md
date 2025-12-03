# HS_Hook - x86 32-bit Lightweight Hook Library

## Overview
A lightweight, third-party dependency-free hook library designed for the x86 32-bit architecture. It supports cross-platform operation (Unix-like and Windows) and is compatible with mainstream compilers (MSVC, GCC, Clang).

## Quick Example
```cpp
#include "HS_Hook.h"
#include <iostream>

// Use HS_NOINLINE to prevent compiler inlining optimization, ensuring the Hook takes effect
static HS_NOINLINE void Original()
{
    std::cout << "Original" << std::endl;
}

static HS_NOINLINE void Relpaced()
{
    std::cout << "Relpaced" << std::endl;

    // Call the original function (recursive calls are handled internally by the library)
    Original();
}

int main()
{
    // Install the hook, replacing Original with Relpaced
    if (!HSLL::HSHook::Install((void*)Original, (void*)Relpaced))
    {
        return -1;
    }

    std::cout << "---------------------" << std::endl;
    Original(); // Outputs "Relpaced\nOriginal\n"

    std::cout << "---------------------" << std::endl;
    Relpaced(); // Outputs "Relpaced\nRelpaced\nOriginal\n"

    // Remove the hook
    if (!HSLL::HSHook::Remove((void*)Original))
    {
        return -1;
    }

    std::cout << "---------------------" << std::endl;
    Original(); // Outputs "Original"

    std::cout << "---------------------" << std::endl;
    Relpaced(); // Outputs "Relpaced\nOriginal\n"

    std::cout << "---------------------" << std::endl;
    return 0;
}
```

## Basic Usage

### Installing a Hook
```cpp
// Returns true on success, false on failure
bool success = HSLL::HSHook::Install((void*)original_function_address, (void*)new_function_address);
```

### Removing a Hook
```cpp
// Returns true on success, false on failure
bool success = HSLL::HSHook::Remove((void*)original_function_address);
```

## Notes
1. **The `Install` and `Remove` functions themselves are not thread-safe. Concurrent calls from multiple threads are not allowed. Also, ensure the target function is not being executed when performing these operations.**
2. **The original function and the replacement function must use the same calling convention.**
3. **If you are also the author of the original function, it is advisable to add the `HS_NOINLINE` attribute or the compiler-specific `noinline` attribute to the target function to prevent inlining optimizations that may cause the Hook to fail.**
4. **Functions with very short bodies (insufficient instruction space) may cause the Hook to fail.**
5. **If the function contains jumps or code that references the function's starting address, installing a Hook may cause exceptions.**