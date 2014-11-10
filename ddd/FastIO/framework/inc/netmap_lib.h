//	$Id: netmap_lib.h 2013-05-23 likunxiang$
//

#ifndef _NM_LIB_H_
#define _NM_LIB_H_

#include <netinet/in.h>
#include "fio_lfqueue.h"

#define BigLittleSwap16(A)  ((((u_int16_t)(A) & 0xff00) >> 8) | \
                            (((u_int16_t)(A) & 0x00ff) << 8))

#define MAX_THD_NUM 30
#define MAX_PAYLOAD_SIZE 1500
#define MAX_PKT_COPY_ONCE 512

#define rf_n ((u_int)0)
#define rf_s ((u_int)0x1)
#define rf_e ((u_int)0x2)

struct traninfo
{
    int size;
    u_int ring_id;
    u_int slot_id;
    u_int range;
    u_int list_len;
    u_char *buf;
};

typedef int (*fun_proc) (int, struct traninfo*, char*);

extern int g_thd_num;
extern int do_shutdown;
extern LFQueue * g_msg_queues[MAX_THD_NUM];
extern pthread_t g_work_thdid[MAX_THD_NUM];
extern int g_bigend;

void init_trans(struct traninfo *trans, int size);
int ef_sync();
int ef_sync2();
int inc_num_copied(u_int r_id, u_int c);
int inc_num_copied2(u_int r_id, u_int s, u_int e, u_int expect_len);
int ef_mainrecvmsg(int fd);
int ef_recvmsg(int fd, struct traninfo *trans, uint32_t size);
int ef_sendmsg(int sockfd, const void *buff, size_t nbytes, int flags);
int ef_send(int sockfd, const void *buff, size_t nbytes, int flags);
void ef_close(int fd);
int ef_bind(const char *device, int port);
void * ef_count_pkt(void *data);
int checkCPUendian();
int init_msg_queues(int q_size, fun_proc fun);
int destroy_msg_queues();

#endif
