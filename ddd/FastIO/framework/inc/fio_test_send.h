//	$Id: fio_test_send.h 2014-04-12 likunxiang $
//
#ifndef __FIO_TEST_SEND_H_
#define __FIO_TEST_SEND_H_

#include "fio_ringbuf.h"
typedef struct fio_txdata *(*fun_get_pkt)(int nic_id);
void fio_test_snd_set_fun(fun_get_pkt _fun);
void * fio_test_send(void *t);
#endif
