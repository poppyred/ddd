#ifndef __LIBWNS_DAEMON_H__
#define __LIBWNS_DAEMON_H__

/**
 * wns_init_daemon - 将进程变成守护进程
 *     脱离终端
 *     关闭标准输入输出，标准错误
 *     将当前工作目录切换到@path
 *     改变默认文件创建权限mask为0
 *
 * @param path 要切换到的当前工作目录。参数为NULL是切换到/tmp
 *
 * @return 0 成功
 *        -1 fork失败
 *        -2 改变当前目录失败
 *
 * @note 返回失败时，标准输入输出不会关闭
 **/

int wns_init_daemon(char *path);

#endif /* __ __LIBWNS_DAEMON_H__ */
