#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/param.h>
#include "libbase_daemon.h"

int wns_init_daemon(char *path)
{
    int pid;
    if ((pid = fork()) > 0)
        exit(0); /* 是父进程，结束父进程 */
    else if (pid < 0)
        return -1; /* fork失败 */

    /* 是第一子进程，后台继续执行 */
    setsid(); /* 第一子进程成为新的会话组长和进程组长 */
    /* 并与控制终端分离 */

    if ((pid = fork()) > 0)
        exit(0); /* 是第一子进程，结束第一子进程 */
    else if (pid < 0)
        return -1; /* fork失败 */

    /* 是第二子进程，继续 */
    /* 第二子进程不再是会话组长 */

    /* 改变工作目录到/tmp */
    if (chdir(path ? path : "/tmp") != 0)
        return -2;

    /* parasoft suppress item BD-RES-INVFREE-1 */
    /* 关闭标准输入输出，标准错误的文件描述符 */
    close(STDIN_FILENO); /* parasoft-suppress BD-RES-INVFREE-1 */
    close(STDOUT_FILENO); /* parasoft-suppress BD-RES-INVFREE-1 */
    close(STDERR_FILENO); /* parasoft-suppress BD-RES-INVFREE-1 */
    /* parasoft unsuppress item BD-RES-INVFREE-1 */

    umask(0); /* 重设文件创建掩模 */

    return 0;
}
