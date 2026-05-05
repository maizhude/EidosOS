#include "main.h"
#include "EidosOSconfig.h"
#include "task.h"
#include "list.h"
#include "timer.h"
#include "sync.h"
/*********************** Global Variables ***********************/

extern TCB_t *currentTCB;	  // 当前正在运行的任务
extern uint32_t currentTicks; // 系统滴答数
extern List_t delayList;		  // 任务延迟链表
extern List_t readyList;		  // 就绪链表
extern List_t timerList;		  // 定时器链表
extern Semaphore_t *TimerTaskNotifySem; // 定时器任务通知信号量
extern TaskHandle_t timerTask;			// 定时器任务句柄
/*-----------------------------------------------------------*/

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

void vPortSVCHandler(void)
{
	__asm volatile(
		"	ldr	r3, pxCurrentTCBConst2		\n" /* Restore the context. */
		"	ldr r1, [r3]					\n" /* Use pxCurrentTCBConst to get the pxCurrentTCB address. */
		"	ldr r0, [r1]					\n" /* The first item in pxCurrentTCB is the task top of stack. */
		"	ldmia r0!, {r4-r11, r14}		\n" /* Pop the registers that are not automatically saved on exception entry and the critical nesting count. */
		"	msr psp, r0						\n" /* Restore the task stack pointer. */
		"	isb								\n"
		"	mov r0, #0 						\n"
		"	msr	basepri, r0					\n"
		"	bx r14							\n"
		"									\n"
		"	.align 4						\n"
		"pxCurrentTCBConst2: .word currentTCB				\n"
	);
}

void vPortSysTickHandler(void)
{
	HAL_IncTick();
	/* USER CODE BEGIN SysTick_IRQn 1 */
	currentTicks++; // 系统滴答数加1
	// 超限清零
	if (currentTicks == 0xFFFFFFFF)
	{
		currentTicks = 0;
	}
	if (timerList.end.next != &timerList.end) // 如果系统滴答数达到了定时器链表中第一个定时器的超时时间
	{	
		Timer_t *timer = (Timer_t *)timerList.end.next->pvOwner;
		if (timer != NULL && timer->expireTime == currentTicks)
		{
			// 通知定时器任务处理超时定时器
			taskNotifyGive(timerTask);
		}
	}
	if (currentTCB != NULL)
	{
		currentTCB->tick_count++;
	}
	// 时间片切换
	if (currentTCB && currentTCB->tick_count >= 1000)
	{
		currentTCB->tick_count = 0;
		taskYield(); // 触发任务切换
	}
	// 根据delayList中的任务延迟时间，更新任务状态
	ListItem_t *currentItem = delayList.end.next;
	while (currentItem != &delayList.end)
	{
		ListItem_t *next = currentItem->next;
		TCB_t *task = (TCB_t *)currentItem->pvOwner;
		if (currentTicks == task->stateListItem.value)
		{
			__disable_irq(); // 进入临界区，禁止中断
			task->state = READY;
			// 从延迟链表中移除当前任务
			vListRemove(&task->stateListItem);
			if (vListIsInList(&task->eventNode.eventListItem)) // 如果任务事件节点还在信号量等待链表中，先移除
			{
				vListRemove(&task->eventNode.eventListItem);
			}
			// 设置优先级
			task->stateListItem.value = task->priority;
			// 插入就绪链表
			vListInsert(&readyList, &task->stateListItem, 0);
			__enable_irq(); // 退出临界区，允许中断
			// 如果新就绪的任务优先级高于当前正在运行的任务，则触发 PendSV 进行任务切换
			if (task->priority > currentTCB->priority)
			{
				taskYield(); // 触发任务切换
			}
		}
		currentItem = next;
	}
}

void vPortPendSVHandler(void)
{
	__asm volatile(
		"	mrs r0, psp							\n" /* 保存堆栈顶指针到r0 */
		"	isb									\n"
		"	ldr	r3, pxCurrentTCBConst			\n" /* 获取当前任务的TCB. */
		"	ldr	r2, [r3]						\n"
		"	stmdb r0!, {r4-r11, r14}			\n" /* 保存核心寄存器. */
		"	str r0, [r2]						\n" /* 保存新的堆栈顶指针到TCB的第一个成员. */
		"	stmdb sp!, {r0, r3}					\n" /* 将当前堆栈顶指针和TCB地址压入主堆栈. */
		"	mov r0, %0 							\n"
		"	cpsid i								\n" /* 关闭中断 */
		"	msr basepri, r0						\n" /* 设置BASEPRI寄存器以屏蔽中断. */
		"	dsb									\n"
		"	isb									\n"
		"	cpsie i								\n" /* 开启中断 */
		"	bl vTaskSwitchContext				\n" /* （核心）切换到下一个任务. */
		"	isb									\n"
		"	mov r0, #0							\n" /* 将BASEPRI寄存器清零以允许所有中断. */
		"	msr basepri, r0						\n"
		"	ldmia sp!, {r0, r3}					\n" /* 从主堆栈弹出新的堆栈顶指针和TCB地址. */
		"	ldr r1, [r3]						\n" /* 从TCB中获取新的堆栈顶指针. */
		"	ldr r0, [r1]						\n" /* 从新的堆栈顶指针获取核心寄存器. */
		"	ldmia r0!, {r4-r11, r14}			\n" /* 恢复核心寄存器. */
		"	msr psp, r0							\n" /* 恢复堆栈顶指针. */
		"	isb									\n"
		"	bx r14								\n" /* 返回到新的任务. */
		"										\n"
		"	.align 4							\n"
		"pxCurrentTCBConst: .word currentTCB	\n" ::"i"(0x0)//todo: 0x50是临界区掩码，优先级0-15，数值越小优先级越高，这里设置为0表示完全屏蔽所有中断，实际使用中可以根据需要调整为允许某些优先级更高的中断打断任务切换
	);
}