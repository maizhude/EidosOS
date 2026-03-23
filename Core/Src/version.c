#include "version.h"
#include <stdio.h>

/**
 * @brief 获取版本字符串
 * @return 版本字符串，例如：V1.0.0
 */
const char* GetVersionString(void)
{
    static char version_str[16];

    snprintf(version_str, sizeof(version_str),
             "V%d.%d.%d",
             VERSION_MAJOR,
             VERSION_MINOR,
             VERSION_PATCH);

    return version_str;
}

/**
 * @brief 打印版本信息
 */
void PrintVersion(void)
{
    printf("=====================================\n");
    printf("        Project Version Info         \n");
    printf("=====================================\n");
    printf("Version : %s\n", GetVersionString());
    printf("=====================================\n");
}