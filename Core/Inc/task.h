#ifndef __TASK_H
#define __TASK_H

#include <stdint.h>

#define MAX_TASKS 3
#define STACK_SIZE 512

typedef enum { READY, RUNNING, BLOCKED } task_state_t;

typedef struct TCB
{
    uint32_t *sp; // 堆栈指针，必须在第一个位置，以便在上下文切换时正确保存和恢复
    uint16_t tick_count; // 任务运行的系统滴答数
    // void (*taskFunc)(void);
    // int priority;
    uint32_t delay; // 任务延迟时间
    task_state_t state;
}TCB_t;

void task_init(TCB_t *tcb, void (*task)(void), uint32_t *stack, int size);
void task_func1();
void task_func2();
void task_idle();
void prvPortStartFirstTask( void );

#endif /* __TASK_H */