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
} List_t;
/*********************** Function Prototypes *****************/

//初始化节点
void vListItemInit(ListItem_t *node);
// 初始化链表
void vListInit(List_t *list);
// 将节点插入链表尾部
void vListInsertEnd(List_t *list, ListItem_t *node);
// 将节点按照顺序插入链表，根据传入的顺序规则判断从大到小还是从小到大
void vListInsert(List_t *list, ListItem_t *node, int ascending);
// 从链表中删除节点
void vListRemove(ListItem_t *node);
// 获取链表头部节点
ListItem_t *vListGetHead(List_t *list);
//判断节点是否在链表中
int vListIsInList(ListItem_t *node);

#endif /* __LIST_H */