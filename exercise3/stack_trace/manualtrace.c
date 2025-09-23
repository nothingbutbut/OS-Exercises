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