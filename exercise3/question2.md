# macOS(Apple Silicon)调用栈回溯
我在这个问题上实现了基于c的macOS调用栈回溯，定位函数的栈指针和返回地址。

## 回顾：课堂上讲过的函数调用过程
函数调用时，在执行`jal`指令之前，会压入函数调用参数，例如下面的例子：
```assembly
# 调用者函数
caller:
    # 准备参数
    li a0, 10       # 第一个参数
    li a1, 20       # 第二个参数
    li a2, 30       # 第三个参数
    
    # 如果参数超过8个，需要使用栈
    addi sp, sp, -8
    sw a3, 0(sp)    # 第9个参数(如果有)
    
    jal ra, callee  # 调用函数，自动将返回地址保存在ra寄存器
    
    addi sp, sp, 8  # 清理栈上的额外参数(如果有)
    
    ret             # 返回到调用者

# 被调用函数
callee:
    # 函数序言
    addi sp, sp, -16        # 分配栈空间
    sd ra, 8(sp)            # 保存返回地址
    sd fp, 0(sp)            # 保存帧指针
    addi fp, sp, 16         # 设置新的帧指针
    
    # 保存callee-saved寄存器
    addi sp, sp, -16
    sd s1, 8(sp)            # 保存s1(callee-saved)
    sd s2, 0(sp)            # 保存s2(callee-saved)
    
    # 函数体(这里可以访问参数)
    # a0-a2包含前三个参数
    # 可以使用栈上的参数(如果有)
    
    # 函数尾声
    ld s2, 0(sp)            # 恢复s2
    ld s1, 8(sp)            # 恢复s1
    addi sp, sp, 16
    
    ld fp, 0(sp)            # 恢复帧指针
    ld ra, 8(sp)            # 恢复返回地址
    addi sp, sp, 16         # 释放栈空间
    
    ret                     # 返回，使用ra中的地址
```
1. 调用者函数负责压入参数到寄存器和栈中。
2. 被调用函数在序言中保存返回地址和帧指针，并设置新的帧指针。
3. 被调用函数保存callee-saved寄存器的值。

返回地址和栈帧指针fp的相对位置：返回地址在fp的上方8字节处，即`fp - 8`（对于RISC-V64）。

从下到上：调用参数->返回地址->旧的fp->callee-saved寄存器-> 局部变量

## macOS的区别
在arm64架构上，函数调用和返回的机制有一些不同。我们列出以下关键点：
1. **寄存器**：
   - 程序计数器 (Program Counter): pc (等价于 %rip)
   - 栈指针 (Stack Pointer): sp
   - 帧指针 (Frame Pointer): x29 (也常被称为 fp)
   - 链接寄存器 (Link Register): x30 (也常被称为 lr)
2. **函数返回地址**：如果当前函数需要调用其他函数（非叶子函数），它会将当前的 lr 和 fp 寄存器的值压入栈中，然后更新 fp 指向新的栈帧底部(和RISC-V64类似)。返回地址在fp的上方8字节处，即`fp - 8`。但是如果是叶子函数（不调用其他函数的函数），它们可以直接使用 lr 寄存器返回，而无需访问栈。

除此以外，Apple Silicon芯片还引入了一项重要的硬件安全特性：指针认证码 (Pointer Authentication Codes, PAC)，但是我在实验中发现直接追踪不考虑这一机制也能成功地打印调用栈。我对这一机制并不了解，猜测要么是我们的实验环境非常简单，要么是PAC机制本身并不影响我们读取栈上的返回地址。

## 实现
Apple官方提供了类似于linux的`backtrace`函数（[官方文档](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/backtrace.3.html)。我们可以借助这个工具轻易地完成调用栈回溯。

```c
#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h> // 和linux没什么区别

#define BACKTRACE_SIZE 20

// 打印函数调用栈
void print_stackframe() {
    void *buffer[BACKTRACE_SIZE];
    int len = backtrace(buffer, BACKTRACE_SIZE); // 获取调用栈的个数
    printf("--- Stack Trace Begin ---\n");
    printf("backtrace() captured %d frames:\n", len);

    // 将地址转换为可读的符号（函数名+偏移量）
    char **symbols = backtrace_symbols(buffer, len);
    if (symbols == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    // 从 j=1 开始打印可以跳过 print_stackframe 本身
    for (int j = 0; j < len; j++) {
        // 格式：[序号] 镜像名 函数地址 函数名 + 偏移
        printf("[%02d] %s\n", j, symbols[j]);
    }
    
    // backtrace_symbols 分配的内存需要手动释放
    free(symbols);
    printf("--- Stack Trace End ---\n");
}

void foo() {
    printf("Inside foo()\n");
    print_stackframe();
}

void bar() {
    printf("Inside bar()\n");
    foo();
}

int main() {
    printf("Inside main()\n");
    bar();
    return 0;
}
```
运行得到的结果如下：
```bash
Inside main()
Inside bar()
Inside foo()
--- Stack Trace Begin ---
backtrace() captured 5 frames:
[00] 0   t                                   0x000000010461fd34 print_stackframe + 44
[01] 1   t                                   0x000000010461fe54 foo + 24
[02] 2   t                                   0x000000010461fe74 bar + 24
[03] 3   t                                   0x000000010461fea4 main + 40
[04] 4   dyld                                0x00000001985bab98 start + 6076
--- Stack Trace End ---
```
可以看到，调用栈被正确地打印出来了。而且有趣的是还有一个`dyld`的调用，这是因为macOS使用动态链接库，`dyld`是动态链接器,负责加载和链接动态库，在加载完成后会调用main函数。

参考往年的优秀回答，我们也可以“古法”追溯，利用汇编代码手动保存和恢复fp和lr寄存器的值，从而实现栈回溯。下面是一个简单的例子：
```c
#include <stdio.h>
#include <stdint.h>

// 内联汇编获取当前的帧指针 (x29)
static inline uint64_t get_fp() {
    uint64_t fp;
    asm volatile ("mov %0, x29" : "=r" (fp));
    return fp;
}

// 内联汇编获取当前的链接寄存器 (x30)，即返回地址
static inline uint64_t get_lr() {
    uint64_t lr;
    asm volatile ("mov %0, x30" : "=r" (lr));
    return lr;
}

void print_stackframe_manual() {
    uint64_t fp = get_fp();
    uint64_t lr = get_lr();
    int depth = 0;

    printf("--- Manual Stack Trace Begin ---\n");
    
    // 第 0 帧: 叶子函数的 fp 和 lr直接读取寄存器
    printf("Frame %d: FP=0x%llx, PC(LR)=0x%llx\n", depth++, fp, lr);

    // 回溯栈帧链
    while (fp != 0) {
        // old_fp 和 old_lr 是相邻存储的
        uint64_t* frame_ptr = (uint64_t*)fp;
        uint64_t prev_fp = frame_ptr[0];
        uint64_t prev_lr = frame_ptr[1];

        if (prev_fp == fp || prev_fp == 0) { // 防止死循环或到达栈顶
             break;
        }

        fp = prev_fp;
        lr = prev_lr;

        printf("Frame %d: FP=0x%llx, PC(LR)=0x%llx\n", depth++, fp, lr);

        if (depth > 20) { // 防止无限循环
            printf("... (trace truncated)\n");
            break;
        }
    }
    
    printf("--- Manual Stack Trace End ---\n");
}

void foo_manual() {
    print_stackframe_manual();
}

void bar_manual() {
    foo_manual();
}

int main() {
    bar_manual();
    return 0;
}
```
运行结果如下：
```bash
--- Manual Stack Trace Begin ---
Frame 0: FP=0x16f87aa30, PC(LR)=0x100587d54
Frame 1: FP=0x16f87aa40, PC(LR)=0x100587e9c
Frame 2: FP=0x16f87aa50, PC(LR)=0x100587eb0
Frame 3: FP=0x16f87aa70, PC(LR)=0x100587ed4
Frame 4: FP=0x16f87b0c0, PC(LR)=0x1985bab98
--- Manual Stack Trace End ---
```
可以看到，我们成功地手动追溯了调用栈，和之前使用 `backtrace` 函数的结果是一致的。