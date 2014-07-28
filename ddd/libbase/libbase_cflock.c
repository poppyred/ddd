/*******************************************************************************
 *                                文件锁C接口的实现
 *******************************************************************************/
#include <sys/file.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>         //for macro PATH_MAX

#include "libbase_cflock.h"

#define LOCK_FILE_SURFIX    ".lock"

#define MAX_PATH_LEN        PATH_MAX


/**
 * @brief:  打开锁文件，在原来文件名的后面，再加一个.lock后缀
 * @param:  file - 文件路径
 * @return: 如果成功返回打开后文件的句柄fd,如果失败返回 -1
 */
static int
open_lock_file(const char *file)
{
    unsigned int len;

    if (file == NULL
            || *file == '\0')
        return -1;

    //如果文件名太长，则直接返回加锁失败
    if (strlen(file) > MAX_PATH_LEN - sizeof(LOCK_FILE_SURFIX))
        return -1;

    char lock_file[MAX_PATH_LEN] = {0};
    strncpy(lock_file, file, sizeof(lock_file));
    len = strlen(lock_file);
    strncat(lock_file, LOCK_FILE_SURFIX, MAX_PATH_LEN-len);

    return open(lock_file, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
}

/**
 * @brief: 初始化，分配f1所需的内存 打开文件列表，获取相应的句柄
 * @param: files - 文件函数列表
           nr -  文件数目
           lock - 文件锁参数
 * @return: 如果成功返回0，失败返回-1
 */
int
flock_init(struct flock_st *lock, const char *files[], int nr)
{
    int i;

    if (lock == NULL)
        return -1;

    lock->nr = 0;

    ///! 初始化fd数组
    lock->fd = (int*)malloc(nr * sizeof(int));
    if (lock->fd == NULL)
        return -1;
    for (i = 0; i < nr; i++)
        lock->fd[i] = -1;


    ///! 打开所有的锁文件
    for (i = 0; i < nr; i++)
    {
        int fd = open_lock_file(files[i]);

        if (fd == -1)
        {
            flock_destroy(lock);
            return -1;
        }

        lock->fd[i] = fd;
        //这里先赋值以免某个文件打不开的时候好调用flock_clear
        lock->nr++;
    }


    return 0;
}


/**
 * @brief: 文件写锁，
 * @param: lock - 文件锁参数
 * @return: 如果成功返回0，失败返回-1
 */
int
flock_wrlock(const struct flock_st *lock)
{
    if (lock == NULL)
        return -1;

    int i;
    for (i = 0; i < lock->nr; i++)
    {
        if (flock(lock->fd[i],LOCK_EX|LOCK_NB) == -1)
        {
            //释放前面已经打开的句柄
            flock_unlock(lock);
            return -1;
        }
    }


    return 0;
}


/**
 * @brief: 如果加锁失败，则重复几次加锁
 * @param: lock      - 文件锁参数
           times     - 重复锁的次数
           interval  - 每次失败后休息的时间,单位,us，1s = 1,000,000us
 * @return: 如果成功返回0，失败返回-1
 */
int
flock_wrlock_repeat(const struct flock_st *lock, int times, int interval)
{
    if (lock == NULL)
        return -1;

    int i;
    int lock_suc = 0;
    int try_times = 0;

    for (i = 0; i < lock->nr; i++)
    {
        while (try_times <= times)
        {
            lock_suc = 0;
            if (flock(lock->fd[i], LOCK_EX|LOCK_NB) == -1)
            {
                if (++try_times > times)
                    break;

                if (interval > 0)
                    usleep(interval);
            }
            else
            {
                lock_suc = 1;
                break;
            }

        }

        if(!lock_suc)
            break;
    }

    if (!lock_suc)
    {
        flock_unlock(lock);
        return -1;
    }


    return 0;
}


/**
 * @brief: 文件读锁
 * @param: lock - 文件锁参数

 * @return: 如果成功返回0，失败返回-1
 */
int
flock_rdlock(const struct flock_st *lock)
{
    if (lock == NULL)
        return -1;

    int i;
    for (i = 0; i < lock->nr; i++)
    {
        if (flock(lock->fd[i], LOCK_SH|LOCK_NB) == -1)
        {
            flock_unlock(lock);
            return -1;
        }
    }


    return 0;
}


/**
 * @brief: 如果加锁失败，则重复几次加锁
 * @param: lock      - 文件锁参数
           times     - 重复锁的次数
           interval  - 每次失败后休息的时间,单位,us，1s = 1,000,000us
 * @return: 如果成功返回0，失败返回-1
 */
int
flock_rdlock_repeat(const struct flock_st *lock, int times, int interval)
{
    if (lock == NULL)
        return -1;

    int i;
    int lock_suc = 0;
    int try_times = 0;

    for (i = 0; i < lock->nr; i++)
    {
        while (try_times <= times)
        {
            lock_suc = 0;
            if (flock(lock->fd[i],LOCK_SH|LOCK_NB) == -1)
            {
                if (++try_times > times)
                    break;

                if (interval > 0)
                    usleep(interval);
            }
            else
            {
                lock_suc = 1;
                break;
            }

        }

        if(!lock_suc)
            break;
    }

    if (!lock_suc)
    {
        flock_unlock(lock);
        return -1;
    }


    return 0;
}


/**
 * @brief: 释放文件锁
 * @param: lock - 文件锁参数
 * @return: 成功返回0，失败返回-1
 */
int
flock_unlock(const struct flock_st *lock)
{
    if (lock == NULL)
        return -1;

    int i;
    for (i = 0; i < lock->nr; i++)
    {
        if(lock->fd[i] != -1)
            flock(lock->fd[i], LOCK_UN);
    }


    return 0;
}


/**
 * @brief: 关闭文件句柄
           释放掉内存
 * @param: lock - 文件锁参数
 * @return: 成功返回0，失败返回-1
 */
int
flock_destroy(struct flock_st *lock)
{
    if (lock == NULL)
        return -1;

    int i;
    for (i = 0; i < lock->nr; i++)
    {
        if(lock->fd[i] != -1)
            close(lock->fd[i]);

        lock->fd[i] = -1;
    }

    lock->nr = 0;
    free(lock->fd);
    lock->fd = NULL;


    return 0;
}

