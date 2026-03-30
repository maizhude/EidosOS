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
/**************** macros  *************************/


/***************************************************/

/***************** 全局变量定义 *************************/

TCB_t * currentTCB = NULL; // 当前正在运行的任务
extern uint32_t currentTicks; // 系统滴答数
extern vList delayList; // 任务延迟链表
extern vList readyList; // 就绪链表

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

//初始化链表节点
void taskListItemInit(TCB_t *task)
{
    vListItemInit(&task->stateListItem);
    vListItemInit(&task->eventListItem);
    task->stateListItem.pvOwner = task; // 设置节点所属任务
    task->eventListItem.pvOwner = task; // 设置节点所属任务
    task->stateListItem.value = task->priority; // 将优先级作为节点值，便于排序
    task->eventListItem.value = 0; // 事件链表节点值暂
}

int task_init(void (*func)(void *), int priority, void *const pvParameters, uint32_t stackSize)
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
        vListInsert(&readyList, &newTask->stateListItem);
    }
    return 0;
    
}
/**
 * @brief 选择下一个任务
 * 
 * @return TCB_t* 
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

void task_delay(uint32_t ticks)
{
    // 设置任务状态为阻塞
    currentTCB->state = BLOCKED;
    // 从就绪链表中移除当前任务
    vListRemove(&currentTCB->stateListItem);
    // 设置节点的延时值为绝对时间，便于在延迟链表中排序
    currentTCB->stateListItem.value = currentTicks + ticks;
    // 插入延迟链表，按照剩余时间排序
    vListInsert(&delayList, &currentTCB->stateListItem);
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
            task_delay(500);
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
            task_delay(500);
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
            task_delay(200);
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
