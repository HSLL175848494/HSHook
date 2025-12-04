# HS_Hook - x86 32-bit Lightweight Hook Library

## Overview  
A lightweight, third-party dependency-free hook library designed for the x86 32-bit architecture. It supports cross-platform operation (Unix-like and Windows) and is compatible with mainstream compilers (MSVC, GCC, Clang).

## Quick Example  
```cpp
// Declare as noinline to prevent compiler inlining optimization that may cause Hook failure  
static HS_NOINLINE void Original()  
{  
    std::cout << "Original" << std::endl;  
}  

static HS_NOINLINE void Relpaced()  
{  
    std::cout << "Relpaced" << std::endl;  
    HSLL::HSHook::Original(Original)();  
}  

int main()  
{  
    if (!HSLL::HSHook::Install((void*)Original, (void*)Relpaced)) // Install Original->Relpaced  
    {  
        return -1;  
    }  

    Original(); // Outputs "Relpaced" and "Original"  

    if (!HSLL::HSHook::Remove((void*)Original)) // Uninstall Original->Relpaced  
    {  
        return -1;  
    }  

    Original(); // Outputs "Original"  
    return 0;  
}  
```

## Basic Usage  

### Install Hook  
```cpp
// Returns true on success, false on failure  
bool success = HSLL::HSHook::Install((void*)original_function_address, (void*)new_function_address);  
```

### Call Original Function  
```cpp
// Call the original function after installing the hook  
HSLL::HSHook::Original(original_function)(original_function_parameters);  
```

### Remove Hook  
```cpp
// Returns true on success, false on failure  
bool success = HSLL::HSHook::Remove((void*)original_function_address);  
```

**Install, Remove, and Original are all thread-safe functions.**  

## Notes  
1. **Ensure the target function is not being called when executing `Install` or `Remove`.**  
2. **The original function and the replacement function must use the same calling convention.**  
3. **If you are also the author of the original function, add the `HS_NOINLINE` attribute or the corresponding compiler's `noinline` attribute to the target function to prevent inlining optimization that may cause Hook failure.**  
4. **Functions with very short bodies (insufficient instruction space) may cause Hook failure.**  
5. **If the function contains internal jumps or references to the function's starting address, it may cause exceptions after Hook is applied.**