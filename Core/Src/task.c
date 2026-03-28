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
/**************** macros  *************************/


/***************************************************/

/***************** 全局变量定义 *************************/

TCB_t* tasks[MAX_TASKS]; // 任务队列，最多10个任务
TCB_t * currentTCB = NULL; // 当前正在运行的任务

void error_func() {
    printf("error_func \r\n");
    while(1);
}
uint32_t *myInitialiseStack(uint32_t *stack_top, void (*taskFunc)(void *), void *pvParameters)
{
    uint32_t *sp = stack_top;

    // ===== 硬件自动恢复 =====
    *(--sp) = 0x01000000;        // xPSR（必须）
    *(--sp) = (uint32_t)taskFunc;          // PC（任务入口）
    *(--sp) = (uint32_t)error_func;        // LR（错误处理函数地址）

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

int task_init(void (*func)(void *), int priority, void *const pvParameters, uint32_t stackSize)
{
    static int currentTaskNum;
    if (currentTaskNum >= MAX_TASKS)
    {
        return -1;
    }
    TCB_t *newTask = malloc(sizeof(TCB_t));
    if (newTask == NULL)
    {
        return -1;
    }
    
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
        newTask->sp = myInitialiseStack(stack_top, func, pvParameters);
        newTask->state = READY;
        newTask->priority = priority;
        tasks[currentTaskNum] = newTask;
    }
    currentTaskNum++;
    return 0;
    
}
/**
 * @brief 选择下一个任务
 * 
 * @return TCB_t* 
 */
void vTaskSwitchContext()
{
    int maxPriority = -1;
    static uint16_t next = 0; // 用于记录下一个任务的索引
    // 寻找就绪任务中最大优先级
    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (tasks[i] && tasks[i]->state == READY)
        {
            if (maxPriority < tasks[i]->priority)
            {
                maxPriority = tasks[i]->priority;
            }
        }
    }
    // 同优先级流转
    for (int i = 0; i < MAX_TASKS; i++)
    {
        next = (next + 1) % MAX_TASKS;
        if (tasks[next] && tasks[next]->state == READY && maxPriority == tasks[next]->priority)
        {
            currentTCB = tasks[next];
            break; // 找到优先级最高且未完成的任务并跳出循环，以继续执行下一个任务
        }
    }
    return;
}

void task_delay(uint32_t ticks)
{
    if (currentTCB != NULL)
    {
        currentTCB->delay = ticks;
        currentTCB->state = BLOCKED; // 设置为阻塞状态

        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // 触发 PendSV 进行任务切换
    }
}

/**
 * @brief 切换任务
 * 
 */
// void yield(void)
// {
//     SCB->ICSR |= (1 << 28); // 触发 PendSV
// }

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

// ====== 示例任务 ======
void task_func1() {
    static int count1 = 0;
    while(1)
    {
        if (count1 == 5000)
        {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
            // HAL_Delay(100);
            printf("Task 1: %d\n", count1);
            task_delay(100);
            count1 = 0;
            // yield(); // 手动切换
        }
        count1++;
    }
}

void task_func2() {
    static int count2 = 0;
    while(1)
    {
        if (count2 == 5000)
        {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
            printf("Task 2: %d\n", count2);
            task_delay(100);
            count2 = 0;
            // yield(); // 手动切换
        }
        count2++;
    }
}

void task_func3() {
    static int count3 = 0;
    while(1)
    {
        if (count3 == 5000)
        {
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
            printf("Task 3: %d\n", count3);
            task_delay(100);
            count3 = 0;
            // yield(); // 手动切换
        }
        count3++;
    }
}

void task_idle() {
    while(1)
    {
        // 空闲任务可以进入低功耗模式
        __WFI(); // 等待中断
    }
}
