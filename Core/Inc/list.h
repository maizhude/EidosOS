#ifndef __LIST_H
#define __LIST_H

/*********************** Includes *****************************/
#include <stdint.h>

/*********************** Macro Definitions *******************/

/*********************** Type Definitions ********************/

struct List;

typedef struct ListItem
{
    uint32_t value;
    struct ListItem *prev;
    struct ListItem *next;
    void *pvOwner;
    struct List *container;
} ListItem_t;

typedef struct List
{
    uint32_t itemNumber;
    ListItem_t *index;
    ListItem_t end;
} vList;
/*********************** Function Prototypes *****************/

//初始化节点
void vListItemInit(ListItem_t *node);
// 初始化链表
void vListInit(vList *list);
// 将节点插入链表尾部
void vListInsertEnd(vList *list, ListItem_t *node);
// 将节点按照值从小到大插入链表
void vListInsert(vList *list, ListItem_t *node);
// 从链表中删除节点
void vListRemove(ListItem_t *node);
// 获取链表头部节点
ListItem_t *vListGetHead(vList *list);

#endif /* __LIST_H */