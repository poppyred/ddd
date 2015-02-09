//	$Id: fio_ringbuf.c 2013-05-23 likunxiang$
//
#include "fio_ringbuf.h"
#include <stdlib.h>
#include <string.h>
#include <netinet/ether.h>      /* ether_aton */
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <assert.h>

#ifdef USE_BF_OCCUPY
#define FIORBF_LOCK(q) while (__sync_lock_test_and_set(&(q)->lock,1)) {}
#define FIORBF_UNLOCK(q) __sync_lock_release(&(q)->lock);
#else
#define FIORBF_LOCK(q) while (0)
#define FIORBF_UNLOCK(q) while(0)
#endif
const int MAX_PAYLOAD_SIZE = MAX_PKT_SIZE-(sizeof(struct ether_header)+sizeof(struct ip)+sizeof(struct udphdr));

//////////////////////////////////////////
//非线程安全

inline int fio_rbf_expand(struct fio_ringbuf *bf, int tid)
{
	struct fio_txdata *nd = calloc(bf->bf_size*RBF_DELTA + 1, sizeof(struct fio_txdata));
    if (!nd)
        return -1;
    FIORBF_LOCK(bf);
	int i, c = bf->bf_size+1, txd_size = sizeof(struct fio_txdata);
    struct fio_txdata *j = bf->bf_head;
	for(i = 0; i < c; i++, j++)
    {
        if (j == bf->bf_end)
            j = bf->bf_bufs;
        memcpy(nd+i, j, txd_size);
    }
	bf->bf_head = nd;
	bf->bf_tail = nd+bf->bf_size;
	bf->bf_size = bf->bf_size*RBF_DELTA;
	bf->bf_end = nd+(bf->bf_size+1);
	free(bf->bf_bufs);
    bf->bf_bufs = nd;

    if (bf->vtbl.realloc_if_buf)
    {
        bf->vtbl.realloc_if_buf(bf, c);
    }

    for (i = c; i < bf->bf_size+1; i++)
    {
        bf->bf_bufs[i].vtbl = bf->bf_head->vtbl;
        bf->bf_bufs[i].vtbl.init(bf, bf->bf_bufs+i, tid, i);
    }
    FIORBF_UNLOCK(bf);
    return 0;
}

void fio_rbf_destroy(struct fio_ringbuf *bf)
{
    if (bf->bf_bufs)
        free(bf->bf_bufs);
    if (bf->buf_if_necessary)
        free(bf->buf_if_necessary);
}

inline int fio_rbf_init(struct fio_ringbuf *bf, int tid, int bf_size, struct _ringbuf_vtbl *rbfvtbl, 
        struct _txdata_vtbl *txdvtbl)
{
    int i;
    bf->vtbl = *rbfvtbl;
    bf->bf_size = bf_size;
    bf->bf_bufs = calloc(bf->bf_size + 1, sizeof(struct fio_txdata));
    bf->bf_head = bf->bf_tail = bf->bf_bufs;
    bf->bf_end = bf->bf_bufs + bf->bf_size + 1;
    //bf->bf_reserve = bf->bf_tail;
    if (bf->vtbl.alloc_if_buf)
        bf->vtbl.alloc_if_buf(bf, bf->bf_size + 1);
    for (i = 0; i < bf->bf_size + 1; i++)
    {
        bf->bf_bufs[i].vtbl = *txdvtbl;
        bf->bf_bufs[i].vtbl.init(bf, bf->bf_bufs+i, tid, i);
    }
#ifdef USE_BF_OCCUPY
    bf->lock = 0;
#endif
    return 0;
}

inline int fio_rbf_empty(struct fio_ringbuf *bf)
{
    return (fio_rbf_occupy(bf) <= 0);
}

inline int _fio_rbf_empty(struct fio_txdata *_head, struct fio_txdata *_tail)
{
    return (_head == _tail);
}

inline int fio_rbf_full(struct fio_ringbuf *bf)
{
    return (fio_rbf_occupy(bf) >= bf->bf_size);
}

inline int fio_rbf_reserve_one(struct fio_ringbuf *bf, struct fio_txdata **m)
{
    if (fio_rbf_full(bf))
        return 0;
    *m = bf->bf_tail;
    return 1;
}

inline int fio_rbf_avail(struct fio_ringbuf *bf)
{
    return (bf->bf_size - fio_rbf_occupy(bf));
}
inline int fio_rbf_size(struct fio_ringbuf *bf)
{
    return bf->bf_size;
}

inline int fio_rbf_occupy(struct fio_ringbuf *bf)
{
#ifdef USE_BF_OCCUPY
    return bf->bf_occupy;
#else
    int ret;
    if (bf->bf_head <= bf->bf_tail) 
        ret = bf->bf_tail - bf->bf_head;
    else
        ret = bf->bf_size - (bf->bf_head-bf->bf_tail-1);
    return ret;
#endif
}

inline int fio_rbf_reserve_all(struct fio_ringbuf *bf, struct fio_txdata **m)
{
    int ret = bf->bf_size - fio_rbf_occupy(bf);
    if (ret > 0)
        *m = bf->bf_tail;
    return ret;
}

inline int fio_rbf_commit(struct fio_ringbuf *bf, int len)
{
    int avail = bf->bf_size - fio_rbf_occupy(bf);
    struct fio_txdata *ptr_v = NULL;
    FIORBF_LOCK(bf);
    if (len > avail)
    {
	    if (avail >= 0)
		    len = avail;
	    else
	    {
		    fprintf(stderr,"<ERROR> avail = %d, occpy = %d, bfsize = %d",
				    avail, fio_rbf_occupy(bf), bf->bf_size);
		    assert(avail >= 0);
		    len = avail = 0;
	    }
    }
    if (bf->bf_head > bf->bf_tail) 
    {
	    //bf->bf_tail += len;
	    ptr_v = bf->bf_tail + len;
    }
    else
    {
	    if (bf->bf_tail + len < bf->bf_end)
	    {
		    //bf->bf_tail += len;
		    ptr_v = bf->bf_tail + len;
	    }
	    else
	    {
		    //bf->bf_tail = bf->bf_bufs + len - (bf->bf_end - bf->bf_tail);
		    ptr_v = bf->bf_bufs + len - (bf->bf_end - bf->bf_tail);
	    }
    }
    assert(ptr_v >= bf->bf_bufs);
    bf->bf_tail = ptr_v;

#ifdef USE_BF_OCCUPY
    bf->bf_occupy += len;
#endif
    FIORBF_UNLOCK(bf);
    return len;
}

inline struct fio_txdata * fio_rbf_seek(struct fio_txdata **_head, struct fio_txdata *_end,
        struct fio_txdata *_txbufs, int num)
{
    if (*_head + num >= _end)
        *_head = _txbufs + num - (_end - *_head); 
    else
        *_head += num;

    return *_head;
}

inline struct fio_txdata * fio_rbf_next(struct fio_txdata *m)
{
    struct fio_ringbuf *bf = (struct fio_ringbuf*)m->parent;
    if (m + 1 == bf->bf_end)
        return bf->bf_bufs; 
    return (m + 1);
}

inline int fio_rbf_consume(struct fio_ringbuf *bf, int num)
{
    FIORBF_LOCK(bf);
    if (fio_rbf_occupy(bf) < num)
        num = fio_rbf_occupy(bf);

    if (bf->bf_head < bf->bf_tail) 
        bf->bf_head += num;
    else
    {
        if (bf->bf_head + num < bf->bf_end)
            bf->bf_head += num;
        else
            bf->bf_head = bf->bf_bufs + num - (bf->bf_end - bf->bf_head);
    }
#ifdef USE_BF_OCCUPY
    bf->bf_occupy -= num;
#endif
    FIORBF_UNLOCK(bf);
    return num;
}

inline int _fio_rbf_consume(struct fio_ringbuf *bf, struct fio_txdata *_head, int num)
{
    FIORBF_LOCK(bf);
    bf->bf_head = _head;
    if ( fio_rbf_occupy(bf) < num )
        num = fio_rbf_occupy(bf);
#ifdef USE_BF_OCCUPY
    bf->bf_occupy -= num;
#endif
    FIORBF_UNLOCK(bf);
    return num;
}

/////////////////////////////////////////
