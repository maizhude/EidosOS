#ifndef __TASK_H
#define __TASK_H
/*********************** Includes *****************************/
#include <stdint.h>
#include "list.h"
#include "queue.h"
/*********************** Macro Definitions *******************/
#define MAX_TASKS 10
#define STACK_SIZE 1024
/*********************** Type Definitions ********************/
typedef enum
{
    READY,
    RUNNING,
    BLOCKED
} task_state_t;

typedef struct eventNode
{
    ListItem_t eventListItem;

    // EventGroup 用
    uint32_t waitBits;
    uint8_t waitMode; // 0表示等待所有事件，1表示等待任一事件

} EventNode_t;

typedef struct TCB
{
    uint32_t *sp;        // 堆栈指针，必须在第一个位置，以便在上下文切换时正确保存和恢复
    uint16_t tick_count; // 任务运行的系统滴答数
    int priority;
    task_state_t state;
    ListItem_t stateListItem; // 用于将任务添加到不同的状态链表中的节点
    EventNode_t eventNode; // 任务等待事件的节点
} TCB_t;

typedef struct event
{
    List_t waitingList;
} Event_t;

/*********************** Function Prototypes *****************/

int taskInit(void (*func)(void *), int priority, void *const pvParameters, uint32_t stackSize);
void taskDelay(uint32_t ticks);
void taskYield(void);
void taskWait(Event_t *event, uint32_t timeout);
TCB_t *taskWake(Event_t *event);
void taskChangePriority(TCB_t *task, uint32_t newPriority);
void taskWakeFromEventGroup(TCB_t *task, EventNode_t *node);
/***************************************************************/

#endif /* __TASK_H */