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
    vList waitingList;
} Event_t;

typedef struct semaphore
{
    int count;
    Event_t event; // 等待该信号量的任务链表
} Semaphore_t;

typedef struct mutex
{
    int locked; // 0表示未锁定，1表示已锁定
    TCB_t *owner; // 当前拥有该互斥锁的任务
    Event_t event; // 等待该互斥锁的任务链表
}Mutex_t;

typedef struct eventGroup
{
    uint32_t eventBits; // 当前事件位，每一位代表一个事件
    Event_t event; // 等待该事件组的任务链表
} EventGroup_t;

typedef struct MessageQueue
{
    Queue_t queue; // 队列结构体
    Event_t sendEvent; // 等待发生数据的任务链表
    Event_t recvEvent; // 等待接收数据的任务链表
} MessageQueue_t;

/*********************** Function Prototypes *****************/

int taskInit(void (*func)(void *), int priority, void *const pvParameters, uint32_t stackSize);
void prvPortStartFirstTask( void );
void taskDelay(uint32_t ticks);
void taskYield(void);
Semaphore_t *semaphoreBinaryInit(void);
void semaphoreWait(Semaphore_t *sem);
void semaphoreSignal(Semaphore_t *sem);
Semaphore_t *semaphoreInit(int initialCount);
void semaphoreBinarySignal(Semaphore_t *sem);
void semaphoreBinaryWait(Semaphore_t *sem);
int semaphoreBinaryWaitTimeout(Semaphore_t *sem, uint32_t timeout);
Mutex_t *mutexInit(void);
void mutexLock(Mutex_t *mutex);
void mutexUnlock(Mutex_t *mutex);
EventGroup_t *eventGroupInit(void);
void eventGroupWaitBits(EventGroup_t *eventGroup, uint32_t waitBits, uint8_t waitMode);
void eventGroupSetBits(EventGroup_t *eventGroup, uint32_t bitsToSet);

#endif /* __TASK_H */