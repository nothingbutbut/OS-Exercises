// buddy_system.h
#ifndef BUDDY_SYSTEM_H
#define BUDDY_SYSTEM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// 链表节点结构
typedef struct ListNode {
    struct ListNode* next;
    struct ListNode* prev;
} ListNode;

// 链表结构
typedef struct LinkedList {
    ListNode head;
    size_t size;
} LinkedList;

// 伙伴系统堆结构
typedef struct {
    // 空闲链表数组，索引是阶数
    LinkedList* free_list;
    
    // 最大阶数
    size_t order;
    
    // 统计信息
    size_t user;       // 用户请求的内存大小
    size_t allocated;  // 实际分配的内存大小
    size_t total;      // 总内存大小
} BuddyHeap;

// 初始化链表
void list_init(LinkedList* list);

// 检查链表是否为空
bool list_is_empty(const LinkedList* list);

// 向链表添加节点
void list_push(LinkedList* list, ListNode* node);

// 从链表中取出节点
ListNode* list_pop(LinkedList* list);

// 遍历链表
ListNode* list_iter(LinkedList* list);

// 从迭代器中移除当前节点
void list_remove(LinkedList* list, ListNode* node);

// 创建并初始化伙伴系统堆
BuddyHeap* buddy_heap_create(size_t order);

// 销毁伙伴系统堆
void buddy_heap_destroy(BuddyHeap* heap);

// 初始化伙伴系统堆（添加内存区域）
void buddy_heap_init(BuddyHeap* heap, uintptr_t start, size_t size);

// 分割块，返回实际分配的大小
static size_t split_block(BuddyHeap* heap, ListNode* block, size_t request_size, size_t alloc_size);

// 分配内存
void* buddy_alloc(BuddyHeap* heap, size_t size, size_t align);

// 释放内存
void buddy_free(BuddyHeap* heap, void* ptr, size_t size, size_t align);

// 获取用户请求的内存大小
size_t buddy_stats_alloc_user(const BuddyHeap* heap);

// 获取实际分配的内存大小
size_t buddy_stats_alloc_actual(const BuddyHeap* heap);

// 获取总内存大小
size_t buddy_stats_total_bytes(const BuddyHeap* heap);

#endif // BUDDY_SYSTEM_H