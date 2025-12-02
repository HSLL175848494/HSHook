# HS_Hook - x86 32-bit Lightweight Hook Library

## Overview
A lightweight hook library specifically designed for the x86 32-bit architecture, capable of running on Unix/Linux and Windows systems, and compatible with MSVC, GCC, and Clang compilers.

## Quick Example
```cpp
#include "HS_Hook.h"
#include <iostream>

// Declared as noinline to prevent compiler inlining optimization from causing Hook failure
static HS_NOINLINE void Original()
{
    std::cout << "Original" << std::endl;
}

static HS_NOINLINE void Relpaced()
{
    std::cout << "Relpaced" << std::endl;
    Original(); // Call the original function (a recursive call will invoke the original function, incrementing the Original count by one each time)
}

int main()
{
    if (!HSLL::HSHook::Install((void*)Original, (void*)Relpaced)) // Install Original->Relpaced
    {
        return -1;
    }

    std::cout << "---------------------" << std::endl;
    Original(); // Outputs "Relpaced"\r\n"Original"\r\n

    std::cout << "---------------------" << std::endl;
    Relpaced(); // Outputs "Relpaced"\r\n"Relpaced"\r\n"Original"\r\n

    if (!HSLL::HSHook::Remove((void*)Original)) // Uninstall Original->Relpaced
    {
        return -1;
    }

    std::cout << "---------------------" << std::endl;
    Original(); // Outputs "Original"

    std::cout << "---------------------" << std::endl;
    Relpaced(); // Outputs "Relpaced"\r\n"Original"\r\n

    std::cout << "---------------------" << std::endl;
    return 0;
}
```

## Basic Usage

### Installing a Hook
```cpp
// Returns true on success, false on failure
bool success = HSLL::HSHook::Install(
    (void*)original_function_address,    // Function to be hooked
    (void*)new_function_address          // Function used for replacement
);
```

### Removing a Hook
```cpp
// Returns true on success, false on failure
bool success = HSLL::HSHook::Remove((void*)original_function_address);
```

## Important Notes
1. **`Install` and `Remove` operations are not thread-safe. Ensure the original function is not being executed when calling them.**
2. **The original function and the replacement function must use exactly the same calling convention.**
3. **The target function must be marked with `HS_NOINLINE` or the compiler-specific `noinline` attribute to prevent inlining optimizations.**
4. **Functions with too short a body (insufficient instruction space) may cause Hook failure.**
5. **If the function contains internal jumps or code that references the function's starting address, exceptions may occur after hooking.**