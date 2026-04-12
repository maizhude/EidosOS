#include "queue.h"
#include <string.h>

/*********************** Macro Definitions ***********************/

/***************************************************************/
/*********************** Type Definitions ***********************/

/***************************************************************/

/*********************** Global Variables ***********************/

/***************************************************************/

/*********************** Static Variables ***********************/

/***************************************************************/

/*********************** Function Prototypes ***********************/

/***************************************************************/

/*********************** Function Implementations ***************/

Queue_t *QueueInit(void *buffer, uint32_t itemSize, uint32_t capacity)
{
    Queue_t *queue = (Queue_t *)malloc(sizeof(Queue_t));
    if (queue != NULL)
    {
        queue->buffer = buffer;
        queue->itemSize = itemSize;
        queue->capacity = capacity;
        queue->head = 0;
        queue->tail = 0;
        queue->count = 0;
    }
    return queue;
}

int QueueEnqueue(Queue_t *queue, const void *item)
{
    if(queue == NULL || item == NULL)
    {
        return -1; // Failure
    }

    if (queue->count < queue->capacity)
    {
        memcpy((char *)queue->buffer + (queue->tail * queue->itemSize), item, queue->itemSize);
        queue->tail = (queue->tail + 1) % queue->capacity;
        queue->count++;
    }
    return 0; // Success
}

int QueueDequeue(Queue_t *queue, void *item)
{
    if(queue == NULL || item == NULL)
    {
        return -1; // Failure
    }
    if (queue->count > 0)
    {
        void *dequeuedItem = (char *)queue->buffer + (queue->head * queue->itemSize);
        queue->head = (queue->head + 1) % queue->capacity;
        queue->count--;
        memcpy(item, dequeuedItem, queue->itemSize);
        return 0; // Success
    }
    return -1; // Failure
}

int QueueIsEmpty(Queue_t *queue)
{
    return queue->count == 0;
}

int QueueIsFull(Queue_t *queue)
{
    return queue->count == queue->capacity;
}

int QueueIsNotEmpty(Queue_t *queue)
{
    return queue->count > 0;
}

int QueueIsNotFull(Queue_t *queue)
{
    return queue->count < queue->capacity;
}

void QueueDestroy(Queue_t *queue)
{
    free(queue);
}

