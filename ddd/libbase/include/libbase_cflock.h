/************************************************************************************
 * @author:     hyb
 * @date:       2011-03-04
 * @brief:      C接口文件锁
 * 1. 加锁的过程为flock_init->flock_wrlock/flock_rdlock->flock_unlock->flock_destroy. 
 * 2. flock_wrlock_repeat/flock_rdlock_repeat用在需要重复加锁的地方，用法和
 *    flock_wrlock/flock_rdlock相同
 * 3. 各函数的lock指针参数有一个实体，使用的时候可以用以下形式来实现
 *                       struct flock_st lock;
 *                       flock_init(&lock,files,nr);
 * 4. 加锁时传进被加锁文件的路径，文件锁会被加锁目录产生一个加.lock后缀的锁文件 
 *************************************************************************************/
#ifndef __C_FILE_LOCK_H__
#define __C_FILE_LOCK_H__

#ifndef __KERNEL__

#ifdef __cplusplus
extern "C"{
#endif


/**
 *       @brief: 文件锁结构，用来存储文件锁句柄
 */
typedef struct flock_st
{
        int        nr; //当前的文件数目
        int       *fd; //存放所有文件锁的句柄
}flock_st;

/**
 * @brief: 初始化，分配f1所需的内存 打开文件列表，获取相应的句柄
 * @param: files - 文件函数列表
           nr -  文件数目
           lock - 文件锁参数
 * @return: 如果成功返回0，失败返回-1
 */
extern int 
flock_init(struct flock_st *lock, const char *files[], int nr);

/**
 * @brief: 文件写锁，
 * @param: lock - 文件锁参数
 * @return: 如果成功返回0，失败返回-1
 */
extern int 
flock_wrlock(const struct flock_st *lock);


/**
 * @brief: 如果加锁失败，则重复几次加锁
 * @param: lock      - 文件锁参数
           times     - 重复锁的次数
           interval  - 每次失败后休息的时间,单位,us，1s = 1,000,000us
 * @return: 如果成功返回0，失败返回-1
 */
extern int 
flock_wrlock_repeat(const struct flock_st *lock, int times, int interval);

/**
 * @brief: 文件读锁
 * @param: lock - 文件锁参数
 * @return: 如果成功返回0，失败返回-1
 */
extern int 
flock_rdlock(const struct flock_st *lock);


/**
 * @brief: 如果加锁失败，则重复几次加锁
 * @param: lock      - 文件锁参数
           times     - 重复锁的次数
           interval  - 每次失败后休息的时间,单位,us，1s = 1,000,000us
 * @return: 如果成功返回0，失败返回-1
 */
extern int 
flock_rdlock_repeat(const struct flock_st *lock, int times, int interval);

/**
 * @brief: 释放文件锁
 * @param: lock - 文件锁参数
 * @return: 成功返回0，失败返回-1
 */
extern int 
flock_unlock(const struct flock_st *lock);

/**
 * @brief: 关闭文件句柄
 *         释放掉内存 
 * @param: lock - 文件锁参数
 * @return: 成功返回0，失败返回-1
 */
extern int 
flock_destroy(struct flock_st *lock);

#ifdef __cplusplus
}
#endif

#endif

#endif

