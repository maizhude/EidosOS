#include <stdlib.h>
#include <string.h>
#include <stdio.h>  
#include "list.h"

vList delayList; // 任务延迟链表
vList readyList; // 就绪链表

//初始化节点
void vListItemInit(ListItem_t *node)
{
    node->value = 0;
    node->prev = NULL;
    node->next = NULL;
    node->pvOwner = NULL;
    node->container = NULL;
}

// 初始化链表
void vListInit(vList *list)
{
    list->itemNumber = 0;
    list->index = NULL;
    list->end.value = 0xFFFFFFFF; // 设置哨兵节点的值为最大
    list->end.prev = &list->end;   // 哨兵节点的前驱和后继都指向自己
    list->end.next = &list->end;
}

// 将节点插入链表尾部
void vListInsertEnd(vList *list, ListItem_t *node)
{
    node->prev = list->end.prev; // 新节点的前驱是当前尾节点
    node->next = &list->end;     // 新节点的后继是哨兵节点
    list->end.prev->next = node; // 当前尾节点的后继指向新节点
    list->end.prev = node;       // 哨兵节点的前驱指向新节点
    node->container = list;      // 设置节点所属链表
    list->itemNumber++;          // 链表元素数量加1
}

// 将节点按照值从大到小插入链表
void vListInsert(vList *list, ListItem_t *node)
{
    ListItem_t *current = list->end.next; // 从头节点开始遍历
    while (current != &list->end && current->value > node->value)
    {
        current = current->next; // 找到第一个值大于等于新节点的节点
    }
    // 将新节点插入到current之前
    node->prev = current->prev;
    node->next = current;
    current->prev->next = node;
    current->prev = node;
    node->container = list; // 设置节点所属链表
    list->itemNumber++;     // 链表元素数量加1
}

// 从链表中删除节点
void vListRemove(ListItem_t *node)
{
    node->prev->next = node->next; // 前驱节点的后继指向当前节点的后继
    node->next->prev = node->prev; // 后继节点的前驱指向当前节点的前驱
    node->container->itemNumber--;   // 链表元素数量减1
    node->container = NULL;          // 清除节点所属链表
}

// 获取链表头部节点
ListItem_t *vListGetHead(vList *list)
{
    if (list->itemNumber == 0)
    {
        return NULL; // 链表为空，返回NULL
    }
    return list->end.next; // 返回头节点（哨兵节点的后继）
}