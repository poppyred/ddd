//	$Id: fio_syslog.h 2013-05-23 likunxiang$
//

#ifndef _FIO_SYSLOG_H_
#define _FIO_SYSLOG_H_

#include <syslog.h>
#include <stdint.h>

#define MAX_SYSLOG_LEN          1024
#define FIO_SYSLOG_FACILITIES       24

struct fio_nic;
void fio_slog_open(uint32_t toip, const char *host_name, const char *module_name, 
        int level, const char *fac);
void fio_slog_snd_format(struct fio_nic *nic, const char *message, ...);
void fio_slog_snd(struct fio_nic *nic, const char *message, int data_len);
void fio_slog_close();

#endif
