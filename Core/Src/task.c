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
#include "task.h"
#include "main.h"
/**************** macros  *************************/


/***************************************************/

/***************** 全局变量定义 *************************/

uint32_t task1_stack[STACK_SIZE];
uint32_t task2_stack[STACK_SIZE];
uint32_t task3_stack[STACK_SIZE];
TCB_t tasks[MAX_TASKS]; // 任务队列，最多10个任务
uint8_t task_count;
TCB_t * currentTCB = NULL; // 当前正在运行的任务

void error_func() {
    printf("error_func \r\n");
    while(1);
}
uint32_t *init_stack(uint32_t *stack_top, void (*task)(void))
{
    uint32_t *sp = stack_top;

    // ===== 硬件自动恢复 =====
    *(--sp) = 0x01000000;        // xPSR（必须）
    *(--sp) = (uint32_t)task;    // PC（任务入口）
    *(--sp) = (uint32_t)error_func;        // LR（错误处理函数地址）

    *(--sp) = 0; // R12
    *(--sp) = 0; // R3
    *(--sp) = 0; // R2
    *(--sp) = 0; // R1
    *(--sp) = 0; // R0
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

void task_init(TCB_t *tcb, void (*task)(void), uint32_t *stack, int size)
{
    uint32_t *stack_top = stack + size;

    tcb->sp = init_stack(stack_top, task);
}
/**
 * @brief 选择下一个任务
 * 
 * @return TCB_t* 
 */
void vTaskSwitchContext()
{
    for(int i = 0; i < MAX_TASKS; i++)
    {
        if(currentTCB == &tasks[i])
        {
            continue; // 跳过当前任务
        }
        if (tasks[i].state == READY)
        {
            currentTCB = &tasks[i];
            break;
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
            // HAL_Delay(100);
            task_delay(100);
            count2 = 0;
            // yield(); // 手动切换
        }
        count2++;
    }
}

void task_idle() {
    while(1)
    {
        // 空闲任务可以进入低功耗模式
        __WFI(); // 等待中断
    }
}
