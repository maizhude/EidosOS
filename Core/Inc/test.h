#ifndef __TEST_H
#define __TEST_H

#include <stdint.h>
#include "list.h"
#include "main.h"

void task_func1();
void task_func2();
void task_func3();
void task_idle();
void timer_init();
void timer_callback(void *arg);

#endif /* __TEST_H */