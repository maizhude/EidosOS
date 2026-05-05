/*********************** Includes *****************************/
#include "timer.h"
#include "list.h"
#include "task.h"
#include "sync.h"
/*********************** Macro Definitions ***********************/

/***************************************************************/
/*********************** Type Definitions ***********************/

/***************************************************************/

/*********************** Global Variables ***********************/
List_t timerList; // 定时器链表
extern uint32_t currentTicks; // 系统滴答数
extern Semaphore_t *TimerTaskNotifySem; // 定时器任务通知信号量
extern TaskHandle_t timerTask; // 定时器任务句柄
/***************************************************************/

/*********************** Static Variables ***********************/

/***************************************************************/

/*********************** Function Prototypes ***********************/

/***************************************************************/

/*********************** Function Implementations ***************/
Timer_t *Timer_Init(void (*callback)(void *), void *arg, uint32_t expireTime, uint8_t reload)
{
    // 初始化定时器结构体
    Timer_t *timer = (Timer_t *)malloc(sizeof(Timer_t));
    if (timer != NULL)
    {
        timer->callback = callback;
        timer->arg = arg;
        timer->expireTime = currentTicks + expireTime;
        timer->reloadTime = expireTime;
        timer->reload = reload;
        vListItemInit(&timer->listItem); // 初始化链表项
        timer->listItem.pvOwner = timer; // 设置链表项所属定时器
    }
    return timer;
}

void TimerTaskInit(void)
{
    // 初始化定时器链表
    vListInit(&timerList);
    timerTask = taskInit(TimerTask, 11, NULL, 512); // 创建定时器任务，优先级最低，栈大小512字节
}

void Timer_Start(Timer_t *timer)
{
    if (timer != NULL)
    {
        // 将定时器插入到定时器链表中，按照超时时间排序
        vListInsert(&timerList, &timer->listItem, 1);
    }
}

void Timer_Stop(Timer_t *timer)
{
    if (timer != NULL)
    {
        // 从定时器链表中移除定时器
        vListRemove(&timer->listItem);
    }
}

void TimerTask()
{
    while(1)
    {
        // 定时器任务，检查定时器链表中的定时器是否超时，并执行回调函数
        ListItem_t *currentItem = timerList.end.next;
        taskNotifyWait(1,1000000); // 等待定时器任务通知信号量
        while (currentItem != &timerList.end)
        {
            ListItem_t *next = currentItem->next; // 先保存下一个节点，避免当前节点被删除后无法访问
            Timer_t *timer = (Timer_t *)currentItem->pvOwner;
            if (currentTicks >= timer->expireTime)
            {
                // 定时器超时，执行回调函数
                timer->callback(timer->arg);
                // 从定时器链表中移除定时器
                vListRemove(currentItem);
                if (timer->reload)
                {
                    // 如果是自动重载定时器，重新设置超时时间
                    timer->expireTime = currentTicks + timer->reloadTime;
                    // 将定时器重新插入到定时器链表中
                    vListInsert(&timerList, &timer->listItem, 1);
                }
                else
                {
                    free(timer); // 释放定时器内存
                }
            }
            currentItem = next; // 继续检查下一个定时器
        }
    }
}