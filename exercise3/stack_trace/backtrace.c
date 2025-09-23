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