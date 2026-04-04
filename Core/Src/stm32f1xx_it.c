/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "task.h"
#include "list.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */
extern TCB_t * currentTCB; // 当前正在运行的任务
extern uint32_t currentTicks; // 系统滴答数
extern vList delayList; // 任务延迟链表
extern vList readyList; // 就绪链表
/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */
	__asm volatile (
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
  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */
  __asm volatile
	(
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
	"pxCurrentTCBConst: .word currentTCB	\n"
	::"i"(0x50)
	);
  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  currentTicks++; // 系统滴答数加1
  // 超限清零
  if (currentTicks == 0xFFFFFFFF)
  {
    currentTicks = 0;
  }
  if (currentTCB != NULL)
  {
    currentTCB->tick_count++;
  }
  // 时间片切换
  if (currentTCB && currentTCB->tick_count >= 1000)
  {
    currentTCB->tick_count = 0;
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
  }
  // 根据delayList中的任务延迟时间，更新任务状态
  ListItem_t *currentItem = delayList.end.next;
  while (currentItem != &delayList.end)
  {
    ListItem_t *next = currentItem->next;
    TCB_t *task = (TCB_t *)currentItem->pvOwner;
    if (currentTicks == task->stateListItem.value)
    {
      task->state = READY;
      // 从延迟链表中移除当前任务
      vListRemove(&task->stateListItem);
      // 设置优先级
      task->stateListItem.value = task->priority;
      // 插入就绪链表
      vListInsert(&readyList, &task->stateListItem);
      // 如果新就绪的任务优先级高于当前正在运行的任务，则触发 PendSV 进行任务切换
      if (task->priority > currentTCB->priority)
      {
        SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // 触发 PendSV 进行任务切换
      }
    }
    currentItem = next;
  }
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
