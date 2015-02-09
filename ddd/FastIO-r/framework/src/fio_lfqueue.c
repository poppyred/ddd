//	$Id: fio_lfqueue.c 2013-05-23 likunxiang$
//
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "fio_lfqueue.h"

#define MQ_LOCK(q) while (__sync_lock_test_and_set(&(q)->lock,1)) {}
#define MQ_UNLOCK(q) __sync_lock_release(&(q)->lock);

const int constDEFAULTQUEUESIZE = 512 ;
const int const_delta_len = 1000;

int  LFQ_inner_count(LFQueue * const _lfq);

int LFQ_expand(LFQueue * const _lfq)
{
	int i;
    void *j;
	void *nd = malloc((_lfq->size+const_delta_len+1)*_lfq->unit_size);
    if (!nd)
        return -1;

    int c = LFQ_inner_count(_lfq);
    j = _lfq->head;
	for(i = 0; i < c; i++, j += _lfq->unit_size)
    {
        if (j == _lfq->dataend)
            j = _lfq->data;
        memcpy(nd+i*_lfq->unit_size, j, _lfq->unit_size);
    }

	_lfq->head = nd;
	_lfq->tail = nd+(c)*_lfq->unit_size;

	_lfq->size += const_delta_len;
	_lfq->dataend = nd+(_lfq->size+1)*_lfq->unit_size;

	free(_lfq->data);
    _lfq->data = nd;
    return 0;
}

int  LFQ_inner_count(LFQueue * const _lfq)
{
	if (_lfq->tail >= _lfq->head)
		return (_lfq->tail - _lfq->head) / _lfq->unit_size;
	else
		return (_lfq->size+1 - (_lfq->head-_lfq->tail)/_lfq->unit_size) ;
}

inline void LFQ_inner_get(LFQueue * const _lfq, void *buf)
{
    memcpy(buf, _lfq->head, _lfq->unit_size);

	_lfq->head += _lfq->unit_size;
	if (_lfq->head >= _lfq->dataend)
		_lfq->head = _lfq->data;
}

inline void LFQ_inner_get2(LFQueue * const _lfq, void *buf, int64_t bsize)
{
    if (_lfq->head < _lfq->tail)
    {
        memcpy(buf, _lfq->head, bsize*_lfq->unit_size);
        _lfq->head += bsize*_lfq->unit_size;
    }
    else
    {
        int64_t down = (int64_t)(_lfq->dataend-_lfq->head);
        int64_t down1 = down;
        int64_t up;
        bsize *= _lfq->unit_size;

        if (down > bsize) 
        {
            down = bsize;
            up = 0;
        }
        else if (down == bsize)
            up = 0;
        else 
        {
            up = bsize - down;
        }

        //printf("buf %p, head %p, down %ld, entrys %ld\n", buf, _lfq->head, down, down/_lfq->unit_size);
        memcpy(buf, _lfq->head, down);
        if (up > 0)
        {
            memcpy(buf+down, _lfq->data, up);
        }

        if (down1 > bsize)
            _lfq->head += down;
        else if (down1 == bsize)
            _lfq->head = _lfq->data;
        else
            _lfq->head = _lfq->data+up;
    }
}

inline int LFQ_inner_put4(LFQueue * const _lfq, const void *data, int64_t num_unit)
{
	if (_lfq->head > _lfq->tail)
	{
		memcpy(_lfq->tail, data, num_unit*_lfq->unit_size);
		_lfq->tail += num_unit*_lfq->unit_size;
	}
	else
	{
		int64_t down = (int64_t)(_lfq->dataend-_lfq->tail);
		int64_t down1 = down;
		int64_t up;
		int64_t bsize = num_unit*_lfq->unit_size;

		if (down > bsize)
		{
			down = bsize;
			up = 0;
		}
		else if (down == bsize)
			up = 0;
		else
		{
			up = bsize - down;
		}
		memcpy(_lfq->tail, data, down);
		if (up > 0)
		{
			memcpy(_lfq->data, data+down, up);
		}

		if (down1 > bsize)
			_lfq->tail += down;
		else if (down1 == bsize)
			_lfq->tail = _lfq->data;
		else
			_lfq->tail = _lfq->data+up;
	}

	return 0 ;
}

inline int LFQ_inner_put2(LFQueue * const _lfq, const void *head, int hsize, const void *data, int dsize)
{
    memcpy(_lfq->tail, head, hsize);
    if (dsize > 0)
        memcpy(_lfq->tail+hsize, data, dsize);

    _lfq->tail += _lfq->unit_size;
    if (_lfq->tail >= _lfq->dataend)
        _lfq->tail = _lfq->data;	

    return 0 ;
}

inline int LFQ_inner_put(LFQueue * const _lfq, const void *newData)
{
    memcpy(_lfq->tail, newData, _lfq->unit_size);

    _lfq->tail += _lfq->unit_size;
    if (_lfq->tail >= _lfq->dataend)
        _lfq->tail = _lfq->data;	

    return 0 ;
}

LFQueue * LFQ_create(int unit_size, int Size) 
{
    int mem_size;
    LFQueue *lfq = malloc(sizeof(LFQueue));
    assert(lfq);
    if (!lfq)
        return NULL;
    bzero(lfq, sizeof(LFQueue));

	lfq->size = Size>0?Size:1;
	lfq->unit_size = unit_size;
    mem_size = unit_size * (lfq->size+1);

	lfq->data = malloc(mem_size);
	assert(lfq->data);
    if (!lfq->data)
    {
        free(lfq);
        return NULL;
    }
    bzero(lfq->data, mem_size);

	lfq->tail = lfq->head = lfq->data;
	lfq->dataend = lfq->data+(lfq->size+1)*unit_size;

    return lfq;
}

void LFQ_destroy(LFQueue *_lfq)
{
    free(_lfq->data);
    free(_lfq);
}

int LFQ_count(LFQueue * const _lfq)
{
	int count = 0;
    MQ_LOCK(_lfq);
	count = LFQ_inner_count(_lfq);
	MQ_UNLOCK(_lfq);
	
	return count;
}

int LFQ_is_empty(LFQueue * const _lfq) 
{
	int ret = 0;

    MQ_LOCK(_lfq);
	ret = (LFQ_inner_count(_lfq) == 0);
	MQ_UNLOCK(_lfq);

	return ret ;
}

int LFQ_is_occupied(LFQueue * const _lfq) 
{
	int ret = 0;

	MQ_LOCK(_lfq);
	ret = ( LFQ_inner_count(_lfq) == _lfq->size );
	MQ_UNLOCK(_lfq);

	return ret ;
}

void LFQ_put(LFQueue * const _lfq, const void *newData)
{
	int ret = -1;

	MQ_LOCK(_lfq);
	if( LFQ_inner_count(_lfq) < _lfq->size )
	{
		ret = LFQ_inner_put(_lfq, newData);
	}
	MQ_UNLOCK(_lfq);

	return ;
}

int LFQ_try_put(int tid, LFQueue * const _lfq, const void *newData)
{
	int ret = -1;

    MQ_LOCK(_lfq);
	if( LFQ_inner_count(_lfq) < _lfq->size )
	{
		ret = LFQ_inner_put(_lfq, newData);
	}
    else
    {
        if ( 0 != LFQ_expand(_lfq))
            printf("tid %d expand failed!\n", tid);
        else if( LFQ_inner_count(_lfq) < _lfq->size )
        {
            printf("tid %d new size %d!\n", tid, _lfq->size);
            ret = LFQ_inner_put(_lfq, newData);
        }
    }
    MQ_UNLOCK(_lfq);

	return ret;
}

int LFQ_try_put2(int tid, LFQueue * const _lfq, const void *head, int hsize, const void *data, int dsize)
{
//    MQ_LOCK(_lfq);
//    MQ_UNLOCK(_lfq);
//    return 0;

	int ret = -1;
    if (hsize + dsize > _lfq->unit_size)
        return ret;

    MQ_LOCK(_lfq);
	if( LFQ_inner_count(_lfq) < _lfq->size )
	{
		ret = LFQ_inner_put2(_lfq, head, hsize, data, dsize);
	}
    else
    {
        if ( 0 != LFQ_expand(_lfq))
            printf("tid %d expand failed!\n", tid);
        else if( LFQ_inner_count(_lfq) < _lfq->size )
        {
            printf("tid %d new size %d!\n", tid, _lfq->size);
            ret = LFQ_inner_put2(_lfq, head, hsize, data, dsize);
        }
    }
    MQ_UNLOCK(_lfq);

	return ret;
}

int LFQ_try_put3(int tid, LFQueue * const _lfq, const void *head, int hsize, const u_char **data, const int *dsize, int entry_size)
{
    //MQ_LOCK(_lfq);
    //MQ_UNLOCK(_lfq);
    //return 0;

    int ret = -1;
    int pos;

    if (entry_size < 1)
        return ret;

    MQ_LOCK(_lfq);
    int unit_size = _lfq->unit_size;
    for (pos = 0; pos < entry_size; pos++)
    {
        if (hsize + dsize[pos] > unit_size)
            continue;

        if( LFQ_inner_count(_lfq) < _lfq->size )
        {
            ret = LFQ_inner_put2(_lfq, head+pos, hsize, data[pos], dsize[pos]);
        }
        else
        {
            if ( 0 != LFQ_expand(_lfq))
            {
                printf("tid %d expand failed!\n", tid);
                break;
            }
            else if( LFQ_inner_count(_lfq) < _lfq->size )
            {
                printf("tid %d new size %d!\n", tid, _lfq->size);
                ret = LFQ_inner_put2(_lfq, head+pos, hsize, data[pos], dsize[pos]);
            }
        }
    }
    MQ_UNLOCK(_lfq);

	return ret;
}

int LFQ_try_put4(int tid, LFQueue * const _lfq, const void *data, int entry_size)
{
    int ret = -1;

    if (entry_size < 1)
        return ret;

    MQ_LOCK(_lfq);
    if( LFQ_inner_count(_lfq) + entry_size < _lfq->size )
    {
	    ret = LFQ_inner_put4(_lfq, data, entry_size);
    }
    else
    {
try_expand:
	    if ( 0 != LFQ_expand(_lfq))
	    {
		    printf("tid %d expand failed!\n", tid);
	    }
	    else if( LFQ_inner_count(_lfq) + entry_size < _lfq->size )
	    {
		    printf("tid %d new size %d!\n", tid, _lfq->size);
		    ret = LFQ_inner_put4(_lfq, data, entry_size);
	    }
	    else
		    goto try_expand;
    }
    MQ_UNLOCK(_lfq);

    return ret;
}

void LFQ_get(LFQueue * const _lfq, void *buf)
{
    MQ_LOCK(_lfq);
    LFQ_inner_get(_lfq, buf);
    MQ_UNLOCK(_lfq);
}

int LFQ_try_get(LFQueue * const _lfq, void *buf)
{
   int ret = -1;

   MQ_LOCK(_lfq);
   if( LFQ_inner_count(_lfq) > 0 )
   {
		LFQ_inner_get(_lfq, buf);
		ret = 0 ;
   }
    MQ_UNLOCK(_lfq);

   return ret;
}

int LFQ_try_get2(LFQueue * const _lfq, void *buf, int entry_size)
{
   int ret = 0;

   MQ_LOCK(_lfq);
   int c;
   if( (c=LFQ_inner_count(_lfq)) > 0 )
   {
       c = (entry_size>c?c:entry_size);
       LFQ_inner_get2(_lfq, buf, c);
       ret = c ;
   }
   MQ_UNLOCK(_lfq);

   return ret;
}

int LFQ_top(LFQueue * const _lfq, void *buf, int buf_size)
{
   if (buf_size < _lfq->unit_size)
       return -1;

   int ret = 0;
   MQ_LOCK(_lfq);
   if( LFQ_inner_count(_lfq) > 0 )
       memcpy(buf, _lfq->head, _lfq->unit_size);
   else 
       ret = -1;
   MQ_UNLOCK(_lfq);

   return ret;
}

