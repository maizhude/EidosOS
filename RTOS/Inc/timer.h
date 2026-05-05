#ifndef __TIMER_H
#define __TIMER_H
/*********************** Includes *****************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
/*********************** Macro Definitions *******************/

/*********************** Type Definitions ********************/
typedef struct Timer {
    void (* callback)(void *arg);   // 定时器回调函数指针
    void *arg;                      // 定时器回调函数参数
    ListItem_t listItem;            // 定时器链表项
    uint32_t expireTime;            // 定时器超时时间（单位：毫秒）
    uint32_t reloadTime;            // 定时器重载时间（单位：毫秒），如果是单次定时器则为0
    uint8_t reload;                 // 是否自动重载，1表示自动重载，0表示单次定时器
} Timer_t;

/***************************************************************/

/*********************** Function Prototypes *****************/
Timer_t *Timer_Init(void (*callback)(void *), void *arg, uint32_t expireTime, uint8_t reload);
void Timer_Start(Timer_t *timer);
void Timer_Stop(Timer_t *timer);
void TimerTask();
void TimerTaskInit(void);

#endif /* __TIMER_H */