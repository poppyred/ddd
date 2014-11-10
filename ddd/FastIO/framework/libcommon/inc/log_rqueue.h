#ifndef __LOGRING_QUEUE__
#define __LOGRING_QUEUE__

#include <sys/types.h>

struct NLog;

typedef struct log_rq_tn{
    int cmd;
    void *ptr;
}log_rq_tn;

typedef struct __ring_queue {
    int __index_start;
    int __index_end;
    int __ring_size;
    int __item_size;
    struct NLog *loger;
    //log_rq_t *__buff;
    void *__buff;
    char __tailbuff[0];
}log_rqueue;

size_t log_rqueue_pre_figure(int size, int item);
void log_rqueue_fix(log_rqueue * ringqueue);
void log_rqueue_init(log_rqueue * ringqueue, int size, int item, void *_buf, struct NLog *loger);
void log_rqueue_free(log_rqueue *ringqueue);
int log_rqueue_top(log_rqueue *ringqueue, void **data);
int log_rqueue_pop(log_rqueue *ringqueue, void *data);
int log_rqueue_push(log_rqueue *ringqueue, void *data);
int log_rqueue_push2(log_rqueue *ringqueue, void *data);
void log_rqueue_init2(log_rqueue * ringqueue, int size, int item);
int log_rqueue_pop2(log_rqueue *ringqueue, void *data);

#define __is_full(ringqueue) \
    (((ringqueue).__index_end + 1) % \
     (ringqueue).__ring_size == (ringqueue).__index_start)
#define __is_empty(ringqueue) \
    ((ringqueue).__index_start == (ringqueue).__index_end)

#endif
