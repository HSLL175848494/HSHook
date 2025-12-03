# HS_Hook - x86 32位轻量级钩子库

## 概述
支持 x86 32 位架构设计的轻量级无第三方依赖的钩子库，支持跨平台（ Unix-like 和 Windows）运行，兼容主流编译器（MSVC、GCC、Clang）

## 快速示例
```cpp
#include "HS_Hook.h"
#include <iostream>

// 使用 HS_NOINLINE 防止编译器内联优化，确保 Hook 生效
static HS_NOINLINE void Original()
{
    std::cout << "Original" << std::endl;
}

static HS_NOINLINE void Relpaced()
{
    std::cout << "Relpaced" << std::endl;

    // 调用原函数（库内部已处理递归调用）
    Original();
}

int main()
{
    // 安装钩子，将 Original 替换为 Relpaced
    if (!HSLL::HSHook::Install((void*)Original, (void*)Relpaced))
    {
        return -1;
    }

    std::cout << "---------------------" << std::endl;
    Original(); // 输出 "Relpaced\nOriginal\n"

    std::cout << "---------------------" << std::endl;
    Relpaced(); // 输出 "Relpaced\nRelpaced\nOriginal\n"

    // 移除钩子
    if (!HSLL::HSHook::Remove((void*)Original))
    {
        return -1;
    }

    std::cout << "---------------------" << std::endl;
    Original(); // 输出 "Original"

    std::cout << "---------------------" << std::endl;
    Relpaced(); // 输出 "Relpaced\nOriginal\n"

    std::cout << "---------------------" << std::endl;
    return 0;
}
```

## 基本用法

### 安装钩子
```cpp
// 安装成功返回 true，失败返回 false
bool success = HSLL::HSHook::Install((void*)原函数地址,(void*)新函数地址);
```

### 移除钩子
```cpp
// 移除成功返回 true，失败返回 false
bool success = HSLL::HSHook::Remove((void*)原函数地址);
```

## 注意事项
1. **`Install` 和 `Remove` 函数本身非线程安全，不允许多线程同时调用。同时需确保执行操作时目标函数未被调用**
2. **原函数与替换函数必须使用相同的调用约定**
3. **若您同时是原函数的编写者，尽可能为目标函数添加 `HS_NOINLINE` 或相应编译器的 `noinline` 属性，防止内联优化导致 Hook 失败**
4. **函数体过短（指令空间不足）可能导致 Hook 失败**
5. **若函数内部存在跳转或引用到函数起始地址的代码，Hook 后可能引发异常**