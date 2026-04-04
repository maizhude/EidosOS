#ifndef __VERSION_H__
#define __VERSION_H__

/**
 * @file    version.h
 * @brief   Project version definitions
 * @author  YourName
 * @date    2026-03-23
 *
 * @version history
Version             | Date          | Author        | Description           

V1.0.00             2026-03-23      whx             1、初始版本，能够通过任务主动Yield切换任务
V1.0.01             2026-03-23      whx             1、增加系统滴答定时器，每秒触发一次任务切换
V1.0.02             2026-03-25      whx             1、增加任务延迟功能，任务可以调用task_delay函数进入阻塞状态，等待指定的系统滴答数后自动切换回就绪状态
V1.0.03             2026-03-26      whx             1、切换初始化任务的方式，由静态的数组改为动态分配内存
                                                    2、解决了动态分配导致Systick_Handler中空指针访问的问题
V1.0.04             2026-03-27      whx             1、增加任务优先级功能，任务在创建时可以指定优先级，调度器会优先调度优先级高的就绪任务
                                                    2、同优先级任务之间采用轮转调度
V1.0.05             2026-03-28      whx             1、实现抢占式调度，当一个高优先级任务变为就绪状态时，立即触发任务切换
V1.1.00             2026-03-29      whx             1、实现主动出让CPU功能，任务可以调用taskYield函数主动触发任务切换
                                                    2、任务调度基本完成
V1.1.01             2026-03-30      whx             1、增加链表实现，任务调度器使用链表管理就绪和延迟任务
                                                    2、解决了任务切换时链表操作导致的系统崩溃问题
V1.1.02             2026-03-31      whx             1、增加信号量机制
 
 * @note
 * 每次版本升级，请在此处追加记录：
 * 1. 修改版本号宏
 * 2. 在上方表格添加一行记录
 */

/* ================= Version Macros ================= */
#define VERSION_MAJOR    1   /* 主版本号 */
#define VERSION_MINOR    1   /* 次版本号 */
#define VERSION_PATCH    1   /* 修订版本号 */

/* ================= Version String ================= */
const char* GetVersionString(void);

/* ================= Print Function ================= */
void PrintVersion(void);

#endif /* __VERSION_H__ */