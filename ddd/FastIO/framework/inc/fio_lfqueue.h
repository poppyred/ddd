//	$Id: fio_lfqueue.h 2013-05-23 likunxiang$
//
#ifndef __LF_Queue_H__
#define __LF_Queue_H__

extern const int constDEFAULTQUEUESIZE;

typedef struct 
{
	void *data;
	int size;
	int unit_size;

	void *head;
	void *tail;
	void *dataend;

	int lock;

}LFQueue;

LFQueue * LFQ_create(int unit_size, int Size) ;
void LFQ_destroy(LFQueue *_lfq);
int LFQ_count(LFQueue * const _lfq);
int LFQ_is_empty(LFQueue * const _lfq);
int LFQ_is_occupied(LFQueue * const _lfq);
void LFQ_put(LFQueue * const _lfq, const void *newData);
int LFQ_try_put(int tid, LFQueue * const _lfq, const void *newData);
int LFQ_try_put2(int tid, LFQueue * const _lfq, const void *head, int hsize, const void *data, int dsize);
int LFQ_try_put3(int tid, LFQueue * const _lfq, const void *head, int hsize, const u_char **data, const int *dsize, int entry_size);
int LFQ_try_put4(int tid, LFQueue * const _lfq, const void *data, int entry_size);
void LFQ_get(LFQueue * const _lfq, void *buf);
int LFQ_try_get(LFQueue * const _lfq, void *buf);
int LFQ_try_get2(LFQueue * const _lfq, void *buf, int entry_size);
int LFQ_top(LFQueue * const _lfq, void *buf, int buf_size);

#endif
