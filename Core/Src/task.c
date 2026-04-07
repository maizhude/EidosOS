/***************************************************************
copyright (c) 2019, All rights reserved.
File name: task.c
Author: whx
Description:
Version: V1.0.0
History:
<author>       <time>       <version>       <description>
whx            2026.3.11    V1.0.0          create file
***************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "task.h"
#include "main.h"
#include "list.h"
/*********************** Macro Definitions ***********************/



/***************************************************************/

/*********************** Type Definitions ***********************/



/***************************************************************/

/*********************** Global Variables ***********************/

TCB_t *currentTCB = NULL;     // 当前正在运行的任务
extern uint32_t currentTicks; // 系统滴答数
extern vList delayList;       // 任务延迟链表
extern vList readyList;       // 就绪链表

/***************************************************************/

/*********************** Static Variables ***********************/



/***************************************************************/

/*********************** Function Prototypes ********************/


/***************************************************************/

/*********************** Function Implementations ***************/
/**
 * @brief 任务失败处理函数
 */
static void errorFunc() {
    printf("errorFunc \r\n");
    while(1);
}
/**
 * @brief 初始化任务栈
 * @param stack_top 栈顶指针
 * @param taskFunc 任务函数指针
 * @param pvParameters 参数指针
 * @return 栈顶指针
 */
uint32_t *myInitialiseStack(uint32_t *stack_top, void (*taskFunc)(void *), void *pvParameters)
{
    uint32_t *sp = stack_top;

    // ===== 硬件自动恢复 =====
    *(--sp) = 0x01000000;        // xPSR（必须）
    *(--sp) = (uint32_t)taskFunc;          // PC（任务入口）
    *(--sp) = (uint32_t)errorFunc;         // LR（错误处理函数地址）

    *(--sp) = 0; // R12
    *(--sp) = 0; // R3
    *(--sp) = 0; // R2
    *(--sp) = 0; // R1
    *(--sp) = (uint32_t)pvParameters; // R0
    *(--sp) = 0XFFFFFFFD; // EXC_RETURN（返回到Thread模式，使用PSP）

    // ===== 软件保存 =====
    *(--sp) = 0; // R11
    *(--sp) = 0; // R10
    *(--sp) = 0; // R9
    *(--sp) = 0; // R8
    *(--sp) = 0; // R7
    *(--sp) = 0; // R6
    *(--sp) = 0; // R5
    *(--sp) = 0; // R4

    return sp;
}

/**
 * @brief 初始化任务链表节点
 * @param task 任务控制块指针
 */
void taskListItemInit(TCB_t *task)
{
    vListItemInit(&task->stateListItem);
    vListItemInit(&task->eventNode.eventListItem);
    task->stateListItem.pvOwner = task; // 设置节点所属任务
    task->eventNode.eventListItem.pvOwner = task; // 设置节点所属任务
    task->stateListItem.value = task->priority; // 将优先级作为节点值，便于排序
    task->eventNode.eventListItem.value = 0; // 事件链表节点值暂
}

/**
 * @brief 初始化任务
 * @param func 任务函数指针
 * @param priority 任务优先级
 * @param pvParameters 参数指针
 * @param stackSize 栈大小
 * @return 初始化结果
 */
int taskInit(void (*func)(void *), int priority, void *const pvParameters, uint32_t stackSize)
{
    // 创建新的任务控制块
    TCB_t *newTask = malloc(sizeof(TCB_t));
    if (newTask == NULL)
    {
        return -1;
    }
    // 分配任务栈
    uint32_t *stack = malloc(stackSize);
    if (stack == NULL)
    {
        free(newTask);
        return -1;
    }
    else
    {
        uint32_t stack_addr = (uint32_t)stack + stackSize;
        uint32_t *stack_top = (uint32_t *)stack_addr;
        // 初始化任务栈
        newTask->sp = myInitialiseStack(stack_top, func, pvParameters);
        newTask->state = READY;
        newTask->priority = priority;
        // 初始化链表节点
        taskListItemInit(newTask);
        // 插入就绪链表
        vListInsert(&readyList, &newTask->stateListItem, 0);
    }
    return 0;
    
}
/**
 * @brief 选择优先级最高的就绪任务进行切换
 * 
 */
void vTaskSwitchContext()
{
    //根据链表头部选择优先级最高的就绪任务
    if (readyList.itemNumber > 0)
    {
        ListItem_t *nextItem = readyList.end.next; // 就绪链表头部节点
        TCB_t *nextTask = (TCB_t *)nextItem->pvOwner; // 获取对应的任务控制块
        currentTCB = nextTask; // 切换到下一个任务
    }
    return;
}

/**
 * @brief 任务延迟函数
 * @param ticks 延迟的系统滴答数
 */
void taskDelay(uint32_t ticks)
{
    // 设置任务状态为阻塞
    currentTCB->state = BLOCKED;
    // 从就绪链表中移除当前任务
    vListRemove(&currentTCB->stateListItem);
    // 设置节点的延时值为绝对时间，便于在延迟链表中排序
    currentTCB->stateListItem.value = currentTicks + ticks;
    // 插入延迟链表，按照剩余时间排序
    vListInsert(&delayList, &currentTCB->stateListItem, 1);
    // 切换到下一个任务
    taskYield();
}

/**
 * @brief 切换任务
 * 
 */
void taskYield(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // 触发 PendSV
}

/**
 * @brief 启动第一个任务
 */
void prvPortStartFirstTask( void )
{
	/* Start the first task.  This also clears the bit that indicates the FPU is
	in use in case the FPU was used before the scheduler was started - which
	would otherwise result in the unnecessary leaving of space in the SVC stack
	for lazy saving of FPU registers. */
	__asm volatile(
					" ldr r0, =0xE000ED08 	\n" /* Use the NVIC offset register to locate the stack. */
					" ldr r0, [r0] 			\n"
					" ldr r0, [r0] 			\n"
					" msr msp, r0			\n" /* Set the msp back to the start of the stack. */
					" mov r0, #0			\n" /* Clear the bit that indicates the FPU is in use, see comment above. */
					" msr control, r0		\n"
					" cpsie i				\n" /* Globally enable interrupts. */
					" cpsie f				\n"
					" dsb					\n"
					" isb					\n"
					" svc 0					\n" /* System call to start first task. */
					" nop					\n"
				);
}

static void eventWait(Event_t *event)
{
    // 删除当前任务在就绪链表中的状态节点
    vListRemove(&currentTCB->stateListItem);
    // 将当前任务事件节点添加到事件的等待链表中,按照优先级排序
    vListInsert((vList *)&event->waitingList, &currentTCB->eventNode.eventListItem, 0);
    // 阻塞当前任务
    currentTCB->state = BLOCKED;
    // 切换到下一个任务
    taskYield();
}

static void eventWaitTimeout(Event_t *event, uint32_t timeout)
{
    // 删除当前任务在就绪链表中的状态节点
    vListRemove(&currentTCB->stateListItem);
    // 将当前任务事件节点添加到事件的等待链表中,按照优先级排序
    vListInsert((vList *)&event->waitingList, &currentTCB->eventNode.eventListItem, 0);
    // 阻塞当前任务
    currentTCB->state = BLOCKED;
    // 设置节点的延时值为绝对时间，便于在延迟链表中排序
    currentTCB->stateListItem.value = currentTicks + timeout;
    // 插入延迟链表，按照剩余时间排序
    vListInsert(&delayList, &currentTCB->stateListItem, 1);
    // 切换到下一个任务
    taskYield();
}

static TCB_t *wakeUpFromEvent(Event_t *event)
{
    if (event->waitingList.itemNumber > 0)
    {
        // 从事件的等待链表中取出一个任务
        ListItem_t *waitingItem = event->waitingList.end.next;
        TCB_t *task = (TCB_t *)waitingItem->pvOwner;
        // 将任务事件节点从等待链表中移除
        vListRemove(waitingItem);
        if (vListIsInList(&task->stateListItem)) // 如果任务状态节点还在延时链表中，先移除
        {
            vListRemove(&task->stateListItem);
        }
        // 设置任务状态为就绪
        task->state = READY;
        task->stateListItem.value = task->priority; // 更新节点值为优先级，便于就绪链表排序
        // 将任务状态节点添加到就绪链表中
        vListInsert(&readyList, &task->stateListItem, 0);
        return task;
    }
    return NULL;
}

/**
 * @brief 初始化信号量
 * @param sem 信号量指针
 * @param count 初始计数值
 */
Semaphore_t *semaphoreInit(int count)
{
    Semaphore_t *sem = malloc(sizeof(Semaphore_t));
    if (sem == NULL)
    {
        return NULL;
    }
    sem->count = count;
    vListInit(&sem->event.waitingList); // 初始化等待链表
    return sem;
}

/**
 * @brief 等待信号量
 * @param sem 信号量指针
 */
void semaphoreWait(Semaphore_t *sem)
{
    __disable_irq(); // 进入临界区，禁止中断
    sem->count--;
    if (sem->count < 0)
    {
        eventWait(&sem->event); // 阻塞当前任务，等待信号量事件
        __enable_irq(); // 退出临界区，允许中断 
    }
    else
    {
        __enable_irq(); // 退出临界区，允许中断
    }
}

/**
 * @brief 释放信号量
 * @param sem 信号量指针
 */
void semaphoreSignal(Semaphore_t *sem)
{
    TCB_t *wakeTask = NULL;
    __disable_irq(); // 进入临界区，禁止中断
    sem->count++;

    if (sem->count <= 0)
    {
        wakeTask = wakeUpFromEvent(&sem->event);
    }
    __enable_irq(); // 退出临界区，允许中断
    // 如果新就绪的任务优先级高于当前正在运行的任务，则触发 PendSV 进行任务切换
    if (wakeTask && wakeTask->priority > currentTCB->priority)
    {
        taskYield(); // 触发任务切换
    }
}

/**
 * @brief 初始化二值信号量，初始计数为0，用于任务间同步
 * @param sem 信号量指针
 */
Semaphore_t *semaphoreBinaryInit(void)
{
    Semaphore_t *sem = semaphoreInit(0); // 二值信号量初始计数为0
    return sem;
}

/**
 * @brief 等待二值信号量
 * @param sem 信号量指针
 */
void semaphoreBinaryWait(Semaphore_t *sem)
{
    semaphoreWait(sem); // 直接调用普通信号量的等待函数
}

/**
 * @brief 释放二值信号量
 * @param sem 信号量指针
 */
void semaphoreBinarySignal(Semaphore_t *sem)
{
    if(sem->count < 1)
    {
        semaphoreSignal(sem); // 直接调用普通信号量的释放函数
    }
}

/**
 * @brief 等待二值信号量，带超时功能
 * @param sem 信号量指针
 * @param timeout 超时时间
 * @return int 1表示成功获取信号量，0表示超时
 */
int semaphoreBinaryWaitTimeout(Semaphore_t *sem, uint32_t timeout)
{
    __disable_irq(); // 进入临界区，禁止中断
    sem->count--;
    if (sem->count < 0)
    {
        eventWaitTimeout(&sem->event, timeout);
        __enable_irq();
        return 0; // 超时返回0
    }
    else
    {
        __enable_irq(); // 退出临界区，允许中断
        return 1; // 成功获取信号量返回1
    }
}

/**
 * @brief 初始化互斥锁
 * @return Mutex_t* 互斥锁指针
 */
Mutex_t *mutexInit(void)
{
    Mutex_t *mutex = (Mutex_t *)malloc(sizeof(Mutex_t));
    if (mutex != NULL)
    {
        mutex->locked = 0;
        mutex->owner = NULL;
        vListInit(&mutex->event.waitingList); // 初始化等待链表
    }
    return mutex;
}

/**
 * @brief 锁定互斥锁
 * @param mutex 互斥锁指针
 */
void mutexLock(Mutex_t *mutex)
{
    if (mutex->locked && mutex->owner == currentTCB)
    {
        // 递归锁定，直接返回
        return;
    }
    __disable_irq(); // 进入临界区，禁止中断
    if (mutex->locked == 0)
    {
        // 获取互斥锁
        mutex->locked = 1;
        mutex->owner = currentTCB;
    }
    else
    {
        eventWait(&mutex->event); // 阻塞当前任务，等待互斥锁事件
        if (currentTCB->priority > mutex->owner->priority)
        {
            // 优先级继承
            mutex->owner->stateListItem.value = mutex->owner->priority; // 更新节点值为新的优先级，便于就绪链表排序
            // 如果互斥锁拥有者在就绪链表中，重新排序
            if (vListIsInList(&mutex->owner->stateListItem))
            {
                vListRemove(&mutex->owner->stateListItem);
                vListInsert(&readyList, &mutex->owner->stateListItem, 0);
            }
        }
    }
    __enable_irq(); // 退出临界区，允许中断
}

/**
 * @brief 解锁互斥锁
 * @param mutex 互斥锁指针
 */
void mutexUnlock(Mutex_t *mutex)
{
    TCB_t *wakeTask = NULL;
    if (mutex->owner != currentTCB)
    {
        // 只有拥有互斥锁的任务才能解锁
        return;
    }
    __disable_irq(); // 进入临界区，禁止中断
    wakeTask = wakeUpFromEvent(&mutex->event);
    if (wakeTask != NULL)
    {
        // 将互斥锁所有权转移给下一个任务
        mutex->owner = wakeTask;
    }
    else
    {
        // 没有等待的任务，直接解锁
        mutex->locked = 0;
        mutex->owner = NULL;
    }
    __enable_irq(); // 退出临界区，允许中断
}

/**
 * @brief 初始化事件组
 * @return EventGroup_t* 事件组指针
 */
EventGroup_t *eventGroupInit(void)
{
    EventGroup_t *eventGroup = (EventGroup_t *)malloc(sizeof(EventGroup_t));
    if (eventGroup != NULL)
    {
        eventGroup->eventBits = 0;
        vListInit(&eventGroup->event.waitingList); // 初始化等待链表
    }
    return eventGroup;
}

/**
 * @brief 等待事件组中的事件
 * @param eventGroup 事件组指针
 * @param waitBits 等待的事件位掩码
 * @param waitMode 等待模式，0表示等待所有事件，1表示等待任一事件
 */
void eventGroupWaitBits(EventGroup_t *eventGroup, uint32_t waitBits, uint8_t waitMode)
{
    currentTCB->eventNode.waitBits = waitBits;
    currentTCB->eventNode.waitMode = waitMode;

    __disable_irq(); // 进入临界区，禁止中断
    eventWait(&eventGroup->event); // 阻塞当前任务，等待事件组事件
    __enable_irq(); // 退出临界区，允许中断
}

/**
 * @brief 设置事件组中的事件位
 * @param eventGroup 事件组指针
 * @param bitsToSet 要设置的事件位掩码
 */
void eventGroupSetBits(EventGroup_t *eventGroup, uint32_t bitsToSet)
{
    TCB_t *wakeTask = NULL;
    __disable_irq(); // 进入临界区，禁止中断
    eventGroup->eventBits |= bitsToSet; // 设置事件位

    // 检查等待链表中的任务是否满足唤醒条件
    ListItem_t *currentItem = eventGroup->event.waitingList.end.next;
    while (currentItem != &eventGroup->event.waitingList.end)
    {
        EventNode_t *waitNode = (EventNode_t *)currentItem;
        TCB_t *task = (TCB_t *)waitNode->eventListItem.pvOwner;
        uint32_t waitBits = waitNode->waitBits;
        uint8_t waitMode = waitNode->waitMode;

        int conditionMet = 0;
        if (waitMode == 0) // 等待所有事件
        {
            conditionMet = ((eventGroup->eventBits & waitBits) == waitBits);
        }
        else // 等待任一事件
        {
            conditionMet = ((eventGroup->eventBits & waitBits) != 0);
        }

        if (conditionMet)
        {
            // 从事件的等待链表中移除当前任务
            vListRemove(&waitNode->eventListItem);
            if (vListIsInList(&task->stateListItem)) // 如果任务状态节点还在延时链表中，先移除
            {
                vListRemove(&task->stateListItem);
            }
            // 设置任务状态为就绪
            task->state = READY;
            task->stateListItem.value = task->priority; // 更新节点值为优先级，便于就绪链表排序
            // 将任务状态节点添加到就绪链表中
            vListInsert(&readyList, &task->stateListItem, 0);
        }
        currentItem = currentItem->next;
    }
    __enable_irq(); // 退出临界区，允许中断
    // 如果新就绪的任务优先级高于当前正在运行的任务，则触发 PendSV 进行任务切换
    if (wakeTask && wakeTask->priority > currentTCB->priority)
    {
        taskYield(); // 触发任务切换
    }
}
/***************************************************************/