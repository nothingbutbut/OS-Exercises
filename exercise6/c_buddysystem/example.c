// buddy_system_example.c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "buddy_system.h"

#define HEAP_SIZE (1024 * 1024) // 1MB 堆
#define MAX_ORDER 16           // 最大阶数

int main() {
    // 创建伙伴系统堆
    BuddyHeap* heap = buddy_heap_create(MAX_ORDER);
    if (!heap) {
        printf("Failed to create buddy heap\n");
        return 1;
    }
    
    // 分配内存区域作为堆
    void* memory = malloc(HEAP_SIZE);
    if (!memory) {
        printf("Failed to allocate memory for heap\n");
        buddy_heap_destroy(heap);
        return 1;
    }
    
    // 初始化堆
    buddy_heap_init(heap, (uintptr_t)memory, HEAP_SIZE);
    printf("Initialized heap with %zu bytes\n", buddy_stats_total_bytes(heap));
    
    // 分配内存
    void* ptr1 = buddy_alloc(heap, 704, sizeof(uintptr_t));
    void* ptr2 = buddy_alloc(heap, 1500, sizeof(uintptr_t));
    void* ptr3 = buddy_alloc(heap, 4000, sizeof(uintptr_t));
    
    printf("Allocated 3 blocks\n");
    printf("User memory: %zu bytes\n", buddy_stats_alloc_user(heap));
    printf("Actual allocated: %zu bytes\n", buddy_stats_alloc_actual(heap));
    
    // 释放内存
    buddy_free(heap, ptr2, 1500, sizeof(uintptr_t));
    printf("Freed one block\n");
    printf("User memory: %zu bytes\n", buddy_stats_alloc_user(heap));
    printf("Actual allocated: %zu bytes\n", buddy_stats_alloc_actual(heap));
    
    // 释放其余内存
    buddy_free(heap, ptr1, 704, sizeof(uintptr_t));
    buddy_free(heap, ptr3, 4000, sizeof(uintptr_t));
    
    // 清理
    buddy_heap_destroy(heap);
    free(memory);
    
    return 0;
}