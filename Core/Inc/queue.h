#ifndef __QUEUE_H
#define __QUEUE_H
/*********************** Includes *****************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
/*********************** Macro Definitions *******************/
#define QUEUE_LENGTH 64
/*********************** Type Definitions ********************/
typedef struct Queue
{
    void *buffer;       // 队列缓冲区
    uint32_t itemSize;  // 每个元素的大小
    uint32_t capacity;  // 队列容量（最大元素数量）
    uint32_t head;      // 队列头索引
    uint32_t tail;      // 队列尾索引
    uint32_t count;     // 当前队列中的元素数量
} Queue_t;

/***************************************************************/

/*********************** Function Prototypes *****************/
Queue_t *QueueInit(void *buffer, uint32_t itemSize, uint32_t capacity);
int QueueEnqueue(Queue_t *queue, const void *item);
int QueueDequeue(Queue_t *queue, void *item);
int QueueIsEmpty(Queue_t *queue);
int QueueIsFull(Queue_t *queue);
int QueueIsNotEmpty(Queue_t *queue);
int QueueIsNotFull(Queue_t *queue);
void QueueDestroy(Queue_t *queue);

#endif /* __QUEUE_H */