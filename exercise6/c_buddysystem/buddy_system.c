// buddy_system.c
#include "buddy_system.h"
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// 计算小于等于n的最大2的幂
static size_t prev_power_of_two(size_t num) {
    size_t power = 1;
    while (power * 2 <= num) {
        power *= 2;
    }
    return power;
}

// 向上取整到2的幂
static size_t next_power_of_two(size_t num) {
    size_t power = 1;
    while (power < num) {
        power *= 2;
    }
    return power;
}

// 计算以2为底的对数
static size_t log2_floor(size_t num) {
    size_t log = 0;
    while (num > 1) {
        num >>= 1;
        log++;
    }
    return log;
}

// 初始化链表
void list_init(LinkedList* list) {
    list->head.next = &list->head;
    list->head.prev = &list->head;
    list->size = 0;
}

// 检查链表是否为空
bool list_is_empty(const LinkedList* list) {
    return list->size == 0;
}

// 向链表添加节点
void list_push(LinkedList* list, ListNode* node) {
    node->next = list->head.next;
    node->prev = &list->head;
    list->head.next->prev = node;
    list->head.next = node;
    list->size++;
}

// 从链表中取出节点
ListNode* list_pop(LinkedList* list) {
    if (list_is_empty(list)) {
        return NULL;
    }
    
    ListNode* node = list->head.next;
    list->head.next = node->next;
    node->next->prev = &list->head;
    list->size--;
    
    return node;
}

// 遍历链表
ListNode* list_iter(LinkedList* list) {
    return list->head.next;
}

// 从迭代器中移除当前节点
void list_remove(LinkedList* list, ListNode* node) {
    if (node == &list->head) {
        return;
    }
    
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->size--;
}

// 创建并初始化伙伴系统堆
BuddyHeap* buddy_heap_create(size_t order) {
    BuddyHeap* heap = (BuddyHeap*)malloc(sizeof(BuddyHeap));
    if (!heap) {
        return NULL;
    }
    
    heap->free_list = (LinkedList*)malloc(order * sizeof(LinkedList));
    if (!heap->free_list) {
        free(heap);
        return NULL;
    }
    
    heap->order = order;
    heap->user = 0;
    heap->allocated = 0;
    heap->total = 0;
    
    for (size_t i = 0; i < order; i++) {
        list_init(&heap->free_list[i]);
    }
    
    return heap;
}

// 销毁伙伴系统堆
void buddy_heap_destroy(BuddyHeap* heap) {
    if (heap) {
        free(heap->free_list);
        free(heap);
    }
}

// 初始化伙伴系统堆（添加内存区域）
void buddy_heap_init(BuddyHeap* heap, uintptr_t start, size_t size) {
    // 避免某些平台上的非对齐访问
    uintptr_t aligned_start = (start + sizeof(uintptr_t) - 1) & ~(sizeof(uintptr_t) - 1);
    uintptr_t aligned_end = (start + size) & ~(sizeof(uintptr_t) - 1);
    
    assert(aligned_start <= aligned_end);
    
    size_t total = 0;
    uintptr_t current_start = aligned_start;
    
    while (current_start + sizeof(uintptr_t) <= aligned_end) {
        uintptr_t lowbit = current_start & (~current_start + 1);
        size_t block_size = MIN(lowbit, prev_power_of_two(aligned_end - current_start));
        
        // 如果块大小的阶数大于最大阶数，则将其分割为更小的块
        size_t order = log2_floor(block_size);
        if (order >= heap->order) {
            order = heap->order - 1;
            block_size = (size_t)1 << order;
        }
        
        total += block_size;
        
        ListNode* node = (ListNode*)current_start;
        list_push(&heap->free_list[order], node);
        
        current_start += block_size;
    }
    
    heap->total += total;
}

// 分割块，返回实际分配的大小
static size_t split_block(BuddyHeap* heap, ListNode* block, size_t request_size, size_t alloc_size) {
    size_t order = log2_floor(alloc_size);
    while (order > 0 && alloc_size / 2 >= request_size) {
        // 将块分成两半
        size_t buddy_size = alloc_size / 2;
        ListNode* buddy = (ListNode*)((uintptr_t)block + buddy_size);
        
        list_push(&heap->free_list[order - 1], buddy);
        
        alloc_size = buddy_size;
        order--;
    }
    if (alloc_size * 3 / 4 >= request_size && order >= 2) {
        // 四等分
        size_t quarter_size = alloc_size / 4;
        ListNode* last_part = (ListNode*)((uintptr_t)block + 3 * quarter_size);
        list_push(&heap->free_list[order - 2], last_part);
        if (request_size - 2 * quarter_size > 0) {
            alloc_size = quarter_size * 2 + split_block(heap, (ListNode*)((uintptr_t)block + 2 * quarter_size), request_size - 2 * quarter_size, quarter_size);
        }
    }
    return alloc_size;
}

// 分配内存
void* buddy_alloc(BuddyHeap* heap, size_t size, size_t align) {
    // 计算需要分配的大小（向上取整到2的幂）
    size_t alloc_size = next_power_of_two(MAX(size, MAX(align, sizeof(uintptr_t))));
    size_t order = log2_floor(alloc_size);
    
    // 确保order在合理范围内
    if (order >= heap->order) {
        return NULL;
    }
    
    // 查找可用的块
    size_t i;
    for (i = order; i < heap->order; i++) {
        if (!list_is_empty(&heap->free_list[i])) {
            break;
        }
    }
    
    if (i == heap->order) {
        // 没有找到可用的块
        return NULL;
    }
    
    // 分割较大的块
    for (; i > order; i--) {
        ListNode* block = list_pop(&heap->free_list[i]);
        
        // 将块分成两半
        size_t buddy_size = (size_t)1 << (i - 1);
        ListNode* buddy = (ListNode*)((uintptr_t)block + buddy_size);
        
        list_push(&heap->free_list[i - 1], buddy);
        list_push(&heap->free_list[i - 1], block);
    }
    
    // 从合适大小的链表中获取块
    //ListNode* block = list_pop(&heap->free_list[order]);
    // 增加第二个要求：如空闲块大于申请块的4/3，对可用空闲块进行四等分，直到得到合适可用空闲块。
    ListNode* block = NULL;
    block = list_pop(&heap->free_list[order]);
    if (!block) return NULL;
    // get true alloc_size
    alloc_size = split_block(heap, block, size, alloc_size);
    
    // 更新统计信息
    heap->user += size;
    heap->allocated += alloc_size;
    
    return (void*)block;
}

// 释放内存
void buddy_free(BuddyHeap* heap, void* ptr, size_t size, size_t align) {
    if (!ptr) {
        return;
    }
    
    // 计算原始分配的大小
    size_t alloc_size = next_power_of_two(MAX(size, MAX(align, sizeof(uintptr_t))));
    size_t order = log2_floor(alloc_size);
    
    // 确保order在合理范围内
    if (order >= heap->order) {
        return;
    }
    
    // 将指针放回空闲列表
    ListNode* block = (ListNode*)ptr;
    // 如果是四等分分配的块，归还时拆成3个1/4块分别归还
    if (alloc_size >= size * 4 / 3 && order >= 2) {
        size_t quarter_size = alloc_size / 4;
        size_t quarter_order = order - 2;
        buddy_free(heap, (void*)((uintptr_t)ptr), quarter_size, align);
        buddy_free(heap, (void*)((uintptr_t)ptr + quarter_size), quarter_size, align);
        // 最后一个块递归归还
        if (size > 2 * quarter_size) buddy_free(heap, (void*)((uintptr_t)ptr + 2 * quarter_size), size - 2 * quarter_size, align);
        return;
    }
    list_push(&heap->free_list[order], block);
    
    // 尝试合并伙伴块
    uintptr_t current_ptr = (uintptr_t)ptr;
    size_t current_order = order;
    
    while (current_order < heap->order - 1) {
        // 计算伙伴块地址
        uintptr_t buddy_addr = current_ptr ^ ((uintptr_t)1 << current_order);
        bool found = false;
        
        // 在当前阶中查找伙伴块
        ListNode* node = list_iter(&heap->free_list[current_order]);
        while (node != &heap->free_list[current_order].head) {
            if ((uintptr_t)node == buddy_addr) {
                // 找到伙伴块，从链表中移除
                ListNode* next = node->next;
                list_remove(&heap->free_list[current_order], node);
                found = true;
                node = next;
                break;
            } else {
                node = node->next;
            }
        }
        
        if (found) {
            // 找到伙伴块，合并它们
            list_remove(&heap->free_list[current_order], (ListNode*)current_ptr);
            
            // 更新当前指针为合并后块的开始地址（取较小的地址）
            current_ptr = MIN(current_ptr, buddy_addr);
            current_order++;
            
            // 将合并后的块放入更高阶的链表
            list_push(&heap->free_list[current_order], (ListNode*)current_ptr);
        } else {
            // 没有找到伙伴块，停止合并
            break;
        }
    }
    
    // 更新统计信息
    heap->user -= size;
    heap->allocated -= alloc_size;
}

// 获取用户请求的内存大小
size_t buddy_stats_alloc_user(const BuddyHeap* heap) {
    return heap->user;
}

// 获取实际分配的内存大小
size_t buddy_stats_alloc_actual(const BuddyHeap* heap) {
    return heap->allocated;
}

// 获取总内存大小
size_t buddy_stats_total_bytes(const BuddyHeap* heap) {
    return heap->total;
}