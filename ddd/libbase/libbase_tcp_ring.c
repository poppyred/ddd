#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include "libbase_tcp_ring.h"
#include "libbase_memory.h"
#include "libbase_strutils.h"
#include "libbase_comm.h"
//#include "libbase_log.h"
#include "libbase_log2.h"

#define TCP_RING_NAMESIZE 32

/**
 * 块数据环形缓冲区共享区结构
 */
struct h_tcp_ring_st {
    char        name[TCP_RING_NAMESIZE];    /**< 缓冲区名称 */

    uint32_t size;  /* the size of the allocated buffer */
    uint32_t in;    /* data is added at offset (in % size) */
    uint32_t out;   /* data is extracted from off. (out % size) */

    int8_t   buffer[0]; /* the buffer holding the data */
} PACK_4;


/* 在指定内存上创建环形缓冲区 ，参照kernel:kfifo*/
h_tcp_ring_st *h_tcp_ring_create(const char *name, size_t size)
{
    h_tcp_ring_st *r;
    size_t ring_size;

    /*
     * round up to the next power of 2, since our 'let the indices
     * wrap' tachnique works only in this case.
     */
    if (size & (size - 1))
    {
        if(size > 0x80000000)
        {
            printf("size > 0x80000000\n");
            return NULL;
        }
        size = roundup_pow_of_two(size);
    }
    if( size == 0)
    {
        printf("size == 0\n");
        return NULL;
    }

    ring_size = size + sizeof(h_tcp_ring_st);

    r = (h_tcp_ring_st *)h_malloc(ring_size);
    if (r == NULL) {
        printf("Cannot alloc memory\n");
        return NULL;
    }

    memset(r, 0, ring_size);
    sprintf_n(r->name, sizeof(r->name), "%s", name);

    r->size = size;
    r->in = r->out = 0;

    return r;
}

/* 参照kernel:kfifo*/
int h_tcp_ring_enqueue(h_tcp_ring_st *fifo, const int8_t *buffer,
        size_t len)
{
    unsigned int l;

    if( len > h_tcp_ring_freecount(fifo))
    {
        return -ENOENT;
    }

    len = WNS_MIN(len, fifo->size - fifo->in + fifo->out);

    /*
     * Ensure that we sample the fifo->out index -before- we
     * start putting bytes into the kfifo.
     */

    //todo:: smp_mb();

    /* first put the data starting from fifo->in to buffer end */
    l = WNS_MIN(len, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(fifo->buffer, buffer + l, len - l);

    /*
     * Ensure that we add the bytes to the kfifo -before-
     * we update the fifo->in index.
     */

    //todo:: smp_wmb();

    fifo->in += len;

    return 0;
}
/* 参照kernel:kfifo*/
int h_tcp_ring_peek(h_tcp_ring_st *fifo, int8_t *buffer, size_t len)
{
    unsigned int l;

    if( len > h_tcp_ring_datacount(fifo))
    {
        return -ENOENT;
    }

    len = WNS_MIN(len, fifo->in - fifo->out);

    /*
     * Ensure that we sample the fifo->in index -before- we
     * start removing bytes from the kfifo.
     */

    //todo::smp_rmb();

    /* first get the data from fifo->out until the end of the buffer */
    l = WNS_MIN(len, fifo->size - (fifo->out & (fifo->size - 1)));
    memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, fifo->buffer, len - l);

    /*
     * Ensure that we remove the bytes from the kfifo -before-
     * we update the fifo->out index.
     */

    //todo::smp_mb();

    //fifo->out += len;

    return 0;
}

/* 参照kernel:kfifo*/
int h_tcp_ring_dequeue(h_tcp_ring_st *fifo, int8_t *buffer, size_t len)
{
    if( h_tcp_ring_peek(fifo,buffer,len) < 0)
    {
        return -ENOENT;
    }
    fifo->out += len;
    return 0;
}
/* 参照kernel:kfifo*/
void h_tcp_ring_erase(h_tcp_ring_st *fifo, size_t len)
{
    len = WNS_MIN(len, fifo->in - fifo->out);

    fifo->out += len;
}

/*空间大小*/
size_t h_tcp_ring_count(const h_tcp_ring_st *fifo)
{
    return  fifo->size;
}
/* 空闲空间大小*/
size_t h_tcp_ring_freecount(const h_tcp_ring_st *fifo)
{
    return fifo->size - h_tcp_ring_datacount(fifo);
}
/* 数据空间大小*/
size_t h_tcp_ring_datacount(const h_tcp_ring_st *fifo)
{
    return fifo->in - fifo->out;
}

void h_tcp_ring_destroy(h_tcp_ring_st *r)
{
    if(r)
    {
        h_free(r);
    }
}

/* copyright see wns_atomic.h */
