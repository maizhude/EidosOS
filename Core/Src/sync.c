/*********************** Includes *****************************/
#include "sync.h"
#include "main.h"
/***************************************************************/

/*********************** Macro Definitions ***********************/

/***************************************************************/

/*********************** Type Definitions ***********************/

/***************************************************************/

/*********************** Global Variables ***********************/
extern TCB_t *currentTCB;     // 当前正在运行的任务
/***************************************************************/

/*********************** Static Variables ***********************/

/***************************************************************/

/*********************** Function Prototypes ********************/

/***************************************************************/

/*********************** Function Implementations ***************/
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
        taskWait(&sem->event, 0); // 阻塞当前任务，等待信号量事件
        __enable_irq();         // 退出临界区，允许中断
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
        wakeTask = taskWake(&sem->event);
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
    if (sem->count < 1)
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
        taskWait(&sem->event, timeout);
        __enable_irq();
        return 0; // 超时返回0
    }
    else
    {
        __enable_irq(); // 退出临界区，允许中断
        return 1;       // 成功获取信号量返回1
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
        taskWait(&mutex->event, 0); // 阻塞当前任务，等待互斥锁事件
        if (currentTCB->priority > mutex->owner->priority)
        {
            // 优先级继承
            taskChangePriority(mutex->owner, currentTCB->priority);
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
    wakeTask = taskWake(&mutex->event);
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

    __disable_irq();               // 进入临界区，禁止中断
    taskWait(&eventGroup->event, 0); // 阻塞当前任务，等待事件组事件
    __enable_irq();                // 退出临界区，允许中断
}

/**
 * @brief 设置事件组中的事件位
 * @param eventGroup 事件组指针
 * @param bitsToSet 要设置的事件位掩码
 */
void eventGroupSetBits(EventGroup_t *eventGroup, uint32_t bitsToSet)
{
    TCB_t *wakeTask = NULL;
    __disable_irq();                    // 进入临界区，禁止中断
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
            taskWakeFromEventGroup(task, waitNode); // 唤醒满足条件的任务
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

/**
 * @brief 清除事件组中的事件位
 * @param eventGroup 事件组指针
 * @param bitsToClear 要清除的事件位掩码
 */
void eventGroupClearBits(EventGroup_t *eventGroup, uint32_t bitsToClear)
{
    __disable_irq();                       // 进入临界区，禁止中断
    eventGroup->eventBits &= ~bitsToClear; // 清除事件位
    __enable_irq();                        // 退出临界区，允许中断
}

/**
 * @brief 初始化消息队列
 * @param itemSize 消息大小
 * @param capacity 队列容量
 * @return MessageQueue_t* 消息队列指针
 */
MessageQueue_t *messageQueueInit(uint32_t itemSize, uint32_t capacity)
{
    MessageQueue_t *msgQueue = (MessageQueue_t *)malloc(sizeof(MessageQueue_t));
    if (msgQueue != NULL)
    {
        void *buffer = malloc(itemSize * capacity);
        if (buffer == NULL)
        {
            free(msgQueue);
            return NULL;
        }
        msgQueue->queue.buffer = buffer;
        msgQueue->queue.itemSize = itemSize;
        msgQueue->queue.capacity = capacity;
        msgQueue->queue.head = 0;
        msgQueue->queue.tail = 0;
        msgQueue->queue.count = 0;
        vListInit(&msgQueue->sendEvent.waitingList); // 初始化发送等待链表
        vListInit(&msgQueue->recvEvent.waitingList); // 初始化接收等待链表
    }
    return msgQueue;
}

/**
 * @brief 向消息队列发送数据
 * @param msgQueue 消息队列指针
 * @param item 要发送的数据指针
 */
int messageQueueSend(MessageQueue_t *msgQueue, const void *item)
{
    if (msgQueue == NULL || item == NULL)
    {
        return -1; // Failure
    }
    __disable_irq(); // 进入临界区，禁止中断
    while (QueueIsFull(&msgQueue->queue))
    {
        // 队列满了，阻塞当前任务等待发送事件
        taskWait(&msgQueue->sendEvent, 0);
    }
    QueueEnqueue(&msgQueue->queue, item);
    if (msgQueue->recvEvent.waitingList.itemNumber > 0)
    {
        // 唤醒一个等待接收数据的任务
        TCB_t *wakeTask = taskWake(&msgQueue->recvEvent);
        if (wakeTask && wakeTask->priority > currentTCB->priority)
        {
            taskYield(); // 触发任务切换
        }
    }
    __enable_irq(); // 退出临界区，允许中断
    return 0;       // Success
}

/**
 * @brief 从消息队列接收数据
 * @param msgQueue 消息队列指针
 * @param item 接收数据的缓冲区指针
 */
int messageQueueReceive(MessageQueue_t *msgQueue, void *item)
{
    if (msgQueue == NULL || item == NULL)
    {
        return -1; // Failure
    }
    __disable_irq(); // 进入临界区，禁止中断
    while (QueueIsEmpty(&msgQueue->queue))
    {
        // 队列空了，阻塞当前任务等待接收事件
        taskWait(&msgQueue->recvEvent, 0);
    }
    QueueDequeue(&msgQueue->queue, item);
    if (msgQueue->sendEvent.waitingList.itemNumber > 0)
    {
        // 唤醒一个等待发送数据的任务
        TCB_t *wakeTask = taskWake(&msgQueue->sendEvent);
        if (wakeTask && wakeTask->priority > currentTCB->priority)
        {
            taskYield(); // 触发任务切换
        }
    }
    __enable_irq(); // 退出临界区，允许中断
    return 0;       // Success
}

int messageQueueSendTimeout(MessageQueue_t *msgQueue, const void *item, uint32_t timeout)
{
    if (msgQueue == NULL || item == NULL)
    {
        return -1; // Failure
    }
    __disable_irq(); // 进入临界区，禁止中断
    while (QueueIsFull(&msgQueue->queue))
    {
        // 队列满了，阻塞当前任务等待发送事件，带超时功能
        taskWait(&msgQueue->sendEvent, timeout);
        if (QueueIsFull(&msgQueue->queue))
        {
            __enable_irq(); // 退出临界区，允许中断
            return -1;      // Timeout
        }
    }
    QueueEnqueue(&msgQueue->queue, item);
    if (msgQueue->recvEvent.waitingList.itemNumber > 0)
    {
        // 唤醒一个等待接收数据的任务
        TCB_t *wakeTask = taskWake(&msgQueue->recvEvent);
        if (wakeTask && wakeTask->priority > currentTCB->priority)
        {
            taskYield(); // 触发任务切换
        }
    }
    __enable_irq(); // 退出临界区，允许中断
    return 0;       // Success
}

int messageQueueReceiveTimeout(MessageQueue_t *msgQueue, void *item, uint32_t timeout)
{
    if (msgQueue == NULL || item == NULL)
    {
        return -1; // Failure
    }
    __disable_irq(); // 进入临界区，禁止中断
    while (QueueIsEmpty(&msgQueue->queue))
    {
        // 队列空了，阻塞当前任务等待接收事件，带超时功能
        taskWait(&msgQueue->recvEvent, timeout);
        if (QueueIsEmpty(&msgQueue->queue))
        {
            __enable_irq(); // 退出临界区，允许中断
            return -1;      // Timeout
        }
    }
    QueueDequeue(&msgQueue->queue, item);
    if (msgQueue->sendEvent.waitingList.itemNumber > 0)
    {
        // 唤醒一个等待发送数据的任务
        TCB_t *wakeTask = taskWake(&msgQueue->sendEvent);
        if (wakeTask && wakeTask->priority > currentTCB->priority)
        {
            taskYield(); // 触发任务切换
        }
    }
    __enable_irq(); // 退出临界区，允许中断
    return 0;       // Success
}