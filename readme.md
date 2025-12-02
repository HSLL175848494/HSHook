# HS_Hook - Lightweight x86 32-bit Hooking Library

## Overview
A lightweight hooking library for the x86 32-bit architecture, designed to run on both Unix-like and Windows systems, and compatible with MSVC, GCC, and Clang compilers.

## Quick Example
```cpp
#include "HS_Hook.h"
#include <iostream>

// Declared as noinline to prevent compiler inlining optimization, which could cause Hook failure
static HS_NOINLINE void Original()
{
	std::cout << "Original" << std::endl;
}

static HS_NOINLINE void Relpaced()
{
	std::cout << "Relpaced" << std::endl;

	// Call the original function (recursion logic is handled internally)
	Original();
}

int main()
{
	if (!HSLL::HSHook::Install((void*)Original, (void*)Relpaced)) // Install Original->Relpaced
	{
		return -1;
	}

	std::cout << "---------------------" << std::endl;
	Original(); // Outputs "Relpaced\r\nOriginal\r\n"

	std::cout << "---------------------" << std::endl;
	Relpaced(); // Outputs "Relpaced\r\nRelpaced\r\nOriginal\r\n"

	if (!HSLL::HSHook::Remove((void*)Original)) // Remove Original->Relpaced
	{
		return -1;
	}

	std::cout << "---------------------" << std::endl;
	Original(); // Outputs "Original"

	std::cout << "---------------------" << std::endl;
	Relpaced(); // Outputs "Relpaced\r\nOriginal\r\n"

	std::cout << "---------------------" << std::endl;
	return 0;
}
```

## Basic Usage

### Installing a Hook
```cpp
// Returns true on success, false on failure
bool success = HSLL::HSHook::Install(
    (void*)original_function_address,    // The function to be hooked
    (void*)new_function_address          // The function to replace it with
);
```

### Removing a Hook
```cpp
// Returns true on success, false on failure
bool success = HSLL::HSHook::Remove((void*)original_function_address);
```

## Important Notes
1. **`Install` and `Remove` operations are not thread-safe. You must ensure the original function is not being executed when calling them.**
2. **The original and replacement functions must use exactly the same calling convention.**
3. **Whenever possible, add the `HS_NOINLINE` attribute (or the compiler's equivalent `noinline` attribute) to the target function to prevent inlining optimization from causing Hook failure.**
4. **Very short function bodies (insufficient instruction space) may cause Hook failure.**
5. **If the function contains jumps or code that references the function's starting address (the instruction replacement space), an exception may occur after Hook installation.**
