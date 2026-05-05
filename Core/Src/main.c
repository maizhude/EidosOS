/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "task.h"
#include <stdio.h>
#include "version.h"
#include "list.h"
#include "test.h"
#include "sync.h"
#include "timer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern TCB_t * currentTCB; // 当前正在运行的任务
uint32_t currentTicks = 0; // 系统滴答数
extern List_t delayList;    // 任务延迟链表
extern List_t readyList;    // 就绪链表
Semaphore_t *semKey;        // 二值信号量
Semaphore_t *TimerTaskNotifySem; // 定时器任务通知信号量
Mutex_t *mutexKey;              // 互斥锁
EventGroup_t *eventGroupKey; // 事件组
TaskHandle_t timerTask; // 定时器任务句柄
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
extern void
prvPortStartFirstTask(void);
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void hardfault_handler_c(uint32_t *stack)
{
    uint32_t r0  = stack[0];
    uint32_t r1  = stack[1];
    uint32_t r2  = stack[2];
    uint32_t r3  = stack[3];
    uint32_t r12 = stack[4];
    uint32_t lr  = stack[5];
    uint32_t pc  = stack[6];
    uint32_t psr = stack[7];

    printf("\n===== HARDFAULT =====\n");
    printf("R0  = 0x%08lX\n", r0);
    printf("R1  = 0x%08lX\n", r1);
    printf("R2  = 0x%08lX\n", r2);
    printf("R3  = 0x%08lX\n", r3);
    printf("R12 = 0x%08lX\n", r12);
    printf("LR  = 0x%08lX\n", lr);
    printf("PC  = 0x%08lX\n", pc);
    printf("PSR = 0x%08lX\n", psr);

    while (1);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, 1000);
    return len;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  // 初始化关闭所有灯
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
  NVIC_SetPriority(PendSV_IRQn, 0xFF);  // 最低
  NVIC_SetPriority(SysTick_IRQn, 0xFE); // 比它高一点
  // 初始化链表
  vListInit(&delayList);
  vListInit(&readyList);
  semKey = semaphoreBinaryInit(); // 初始化二值信号量
  TimerTaskNotifySem = semaphoreBinaryInit(); // 初始化定时器任务通知信号量
  mutexKey = mutexInit(); // 初始化互斥锁
  eventGroupKey = eventGroupInit(); // 初始化事件组
  TimerTaskInit(); // 初始化定时器任务
  // 创建任务
  taskInit(task_func1, 2, NULL, STACK_SIZE);
  taskInit(task_func2, 3, NULL, STACK_SIZE);
  taskInit(task_func3, 4, NULL, STACK_SIZE);
  taskInit(task_idle, 1, NULL, STACK_SIZE);
  timer_init();
  HAL_Delay(100);
  PrintVersion();                                 // 打印版本信息
  currentTCB = readyList.end.next->next->pvOwner; // 设置当前任务为第一个就绪的任务
  prvPortStartFirstTask();                        // 启动第一个任务
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
