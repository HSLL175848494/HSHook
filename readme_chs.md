# HS_Hook - x86 32位轻量级钩子库

## 概述
一个用于 x86 32 位架构的轻量级钩子库，支持在 Unix/Linux 与 Windows 系统上运行，并兼容 MSVC、GCC 和 Clang 编译器。

## 快速示例
```cpp
#include "HS_Hook.h"
#include <iostream>

// 声明为 noinline，防止编译器内联优化导致 Hook 失败
static HS_NOINLINE void Original()
{
    std::cout << "Original" << std::endl;
}

static HS_NOINLINE void Relpaced()
{
    std::cout << "Relpaced" << std::endl;

    // 调用原函数(内部已处理递归逻辑)
    Original();
}

int main()
{
    if (!HSLL::HSHook::Install((void*)Original, (void*)Relpaced)) // 安装Original->Relpaced
    {
        return -1;
    }

    std::cout << "---------------------" << std::endl;
    Original(); // 输出 "Relpaced"\r\n"Original"\r\n

    std::cout << "---------------------" << std::endl;
    Relpaced(); // 输出 "Relpaced"\r\n"Relpaced"\r\n"Original"\r\n

    if (!HSLL::HSHook::Remove((void*)Original)) // 卸载Original->Relpaced
    {
        return -1;
    }

    std::cout << "---------------------" << std::endl;
    Original(); // 输出 "Original""\r\n

    std::cout << "---------------------" << std::endl;
    Relpaced(); // 输出 "Relpaced"\r\n"Original"\r\n

    std::cout << "---------------------" << std::endl;
    return 0;
}
```

## 基本用法

### 安装钩子
```cpp
// 安装成功返回 true，失败返回 false
bool success = HSLL::HSHook::Install(
    (void*)原函数地址,    // 需要被 Hook 的函数
    (void*)新函数地址     // 用于替换的函数
);
```

### 移除钩子
```cpp
// 移除成功返回 true，失败返回 false
bool success = HSLL::HSHook::Remove((void*)原函数地址);
```

## 注意事项
1. **`Install` 与 `Remove` 操作非线程安全，调用时必须确保原函数不会被执行**
2. **原函数与替换函数必须使用完全相同的调用约定**：
3. **目标函数必须使用 `HS_NOINLINE` 或相应编译器的 `noinline` 属性，防止内联优化**：
4. **函数体过短（指令空间不足）可能导致 Hook 失败**：
5. **若函数内部存在跳转或引用到函数起始地址的代码，Hook 后可能引发异常**：

