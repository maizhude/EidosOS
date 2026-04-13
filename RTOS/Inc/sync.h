#ifndef __SYNC_H
#define __SYNC_H

/*********************** Includes *****************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "task.h"
/*********************** Macro Definitions *******************/

/***************************************************************/
/*********************** Type Definitions *********************/
typedef struct semaphore
{
    int count;
    Event_t event; // 等待该信号量的任务链表
} Semaphore_t;

typedef struct mutex
{
    int locked;    // 0表示未锁定，1表示已锁定
    TCB_t *owner;  // 当前拥有该互斥锁的任务
    Event_t event; // 等待该互斥锁的任务链表
} Mutex_t;

typedef struct eventGroup
{
    uint32_t eventBits; // 当前事件位，每一位代表一个事件
    Event_t event;      // 等待该事件组的任务链表
} EventGroup_t;

typedef struct MessageQueue
{
    Queue_t queue;     // 队列结构体
    Event_t sendEvent; // 等待发生数据的任务链表
    Event_t recvEvent; // 等待接收数据的任务链表
} MessageQueue_t;
/***************************************************************/
/*********************** Global Variables ***********************/

/***************************************************************/
/*********************** Static Variables ***********************/

/***************************************************************/
/*********************** Function Prototypes ***********************/
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
MessageQueue_t *messageQueueInit(uint32_t itemSize, uint32_t capacity);
int messageQueueSend(MessageQueue_t *msgQueue, const void *item);
int messageQueueReceive(MessageQueue_t *msgQueue, void *item);
int messageQueueSendTimeout(MessageQueue_t *msgQueue, const void *item, uint32_t timeout);
int messageQueueReceiveTimeout(MessageQueue_t *msgQueue, void *item, uint32_t timeout);

#endif /* __SYNC_H */