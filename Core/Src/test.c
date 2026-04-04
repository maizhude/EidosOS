#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "list.h"

extern Semaphore_t *semKey;        // 二值信号量

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
        if (count1 == 5000)
        {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
            // HAL_Delay(100);
            printf("Task 1: %d\n", count1);
            taskDelay(500);
            count1 = 0;
            // yield(); // 手动切换
        }
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET)
        {
            semaphoreBinarySignal(semKey); // 释放信号量，唤醒等待的任务
        }
        count1++;
    }
}

void task_func2()
{
    static int count2 = 0;
    while (1)
    {
        semaphoreBinaryWait(semKey);// 等待信号量
        
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
        printf("Task 2: %d\n", count2);
        // taskDelay(500);
        count2 = 0;
        // yield(); // 手动切换
        
        count2++;
    }
}

void task_func3()
{
    static int count3 = 0;
    while (1)
    {
        if (count3 == 5000)
        {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
            printf("Task 3: %d\n", count3);
            taskDelay(200);
            count3 = 0;
            // yield(); // 手动切换
        }
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