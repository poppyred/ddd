//	$Id: fio_statistics.h 2013-05-23 likunxiang$
//
#ifndef _FIO_STATISTICS_H_
#define _FIO_STATISTICS_H_
#include <stdint.h>
extern struct timeval fio_now;
extern uint32_t fio_gnow;

void * fio_count(void *data);
#endif
