#ifndef __TASK_H
#define __TASK_H

#include <stdint.h>
#include "list.h"
#define MAX_TASKS 10
#define STACK_SIZE 1024

typedef enum { READY, RUNNING, BLOCKED } task_state_t;

typedef struct TCB
{
    uint32_t *sp; // 堆栈指针，必须在第一个位置，以便在上下文切换时正确保存和恢复
    uint16_t tick_count; // 任务运行的系统滴答数
    int priority;
    task_state_t state;
    ListItem_t stateListItem; // 用于将任务添加到不同的状态链表中的节点
    ListItem_t eventListItem; // 用于将任务添加到事件链表中的节点
}TCB_t;

int task_init(void (*func)(void *), int priority, void *const pvParameters, uint32_t stackSize);
void task_func1();
void task_func2();
void task_func3();
void task_idle();
void prvPortStartFirstTask( void );
void taskYield(void);

#endif /* __TASK_H */