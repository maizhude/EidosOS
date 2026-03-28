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
 
 * @note
 * 每次版本升级，请在此处追加记录：
 * 1. 修改版本号宏
 * 2. 在上方表格添加一行记录
 */

/* ================= Version Macros ================= */
#define VERSION_MAJOR    1   /* 主版本号 */
#define VERSION_MINOR    0   /* 次版本号 */
#define VERSION_PATCH    3   /* 修订版本号 */

/* ================= Version String ================= */
const char* GetVersionString(void);

/* ================= Print Function ================= */
void PrintVersion(void);

#endif /* __VERSION_H__ */