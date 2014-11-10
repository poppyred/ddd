#ifndef _FIO_LOG_H_
#define _FIO_LOG_H_

#include <stdio.h>
#include <string.h>
#include "fio_nm_util.h"

#define FIO_LOG_PRLOCK(q) while (__sync_lock_test_and_set(&(q),1)) {}
#define FIO_LOG_PRUNLOCK(q) __sync_lock_release(&(q))
#define LOG_MAX_LINE 1000000

struct fio_log;
struct fio_log_vtbl
{
    void (*print)(struct fio_log *fl, const char *format, ...);
    int (*open)(struct fio_log *fl);
    void (*close)(struct fio_log *fl);
    void (*backup)(struct fio_log *fl);
};

struct fio_log
{
    struct fio_log_vtbl vtbl;
    FILE *m_fp;
    int lock;
    int m_maxlines;
    int m_lines;
    char m_logpath[MAX_FIO_PATH]; 
    char m_fname[MAX_FIO_PATH]; 
};
int fio_log_init(struct fio_log *fl, char *logpath, char *fname);

#endif
