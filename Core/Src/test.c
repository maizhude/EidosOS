#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "list.h"

extern Semaphore_t *semKey;        // 二值信号量
extern Mutex_t *mutexKey;              // 互斥锁
extern EventGroup_t *eventGroupKey; // 事件组
//按键获取函数
int getKeyState()
{
    return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET;
}

// ====== 示例任务 ======
void task_func1()
{
    static int count1 = 0;
    while (1)
    {
        mutexLock(mutexKey); // 获取互斥锁
        if (count1 == 5000)
        {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
            // HAL_Delay(100);
            printf("Task 1: %d\n", count1);
            taskDelay(500);
            count1 = 0;
            // yield(); // 手动切换
        }
        count1++;
        mutexUnlock(mutexKey); // 释放互斥锁
    }
}

void task_func2()
{
    while (1)
    {
        mutexLock(mutexKey); // 获取互斥锁
        // 访问共享资源
        printf("Task with mutex is running.\n");
        taskDelay(10000);     // 模拟任务执行时间
        printf("Task with mutex is running.\n");
        mutexUnlock(mutexKey); // 释放互斥锁
        eventGroupSetBits(eventGroupKey, 0x04); // 设置事件组中的事件位0x04
        printf("Task with mutex is running.\n");
        taskDelay(10000);
    }
}

void task_func3()
{
    static int count3 = 0;
    while (1)
    {
        eventGroupWaitBits(eventGroupKey, 0x05, 0); // 等待事件组中的事件位0x01，等待任一事件
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
        printf("Task 3: %d\n", count3);
        taskDelay(500);
        count3 = 0;
        // yield(); // 手动切换
        count3++;
    }
}

void task_idle()
{
    while (1)
    {
        // 空闲任务可以进入低功耗模式
        __WFI(); // 等待中断
    }
}