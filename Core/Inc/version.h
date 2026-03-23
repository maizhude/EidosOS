#ifndef __VERSION_H__
#define __VERSION_H__

/**
 * @file    version.h
 * @brief   Project version definitions
 * @author  YourName
 * @date    2026-03-23
 *
 * @version history
Version    | Date       | Author    | Description           

V1.0.0      2026-03-23    whx         Initial version

 
 * @note
 * 每次版本升级，请在此处追加记录：
 * 1. 修改版本号宏
 * 2. 在上方表格添加一行记录
 */

/* ================= Version Macros ================= */
#define VERSION_MAJOR    1   /* 主版本号 */
#define VERSION_MINOR    0   /* 次版本号 */
#define VERSION_PATCH    0   /* 修订版本号 */

/* ================= Version String ================= */
const char* GetVersionString(void);

/* ================= Print Function ================= */
void PrintVersion(void);

#endif /* __VERSION_H__ */