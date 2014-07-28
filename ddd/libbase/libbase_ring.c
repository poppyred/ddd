/*=============================================================================
* @brief                                                                      *
* 环形队列数据接口api                                                         *
*                                                                             *
*                                                                             *
*                                         @作者:hyb      @date 2013/04/10     *
==============================================================================*/



#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "libbase_list.h"
#include "libbase_type.h"
#include "libbase_config.h"
#include "libbase_comm.h"
#include "libbase_memory.h"
#include "libbase_strutils.h"
#include "libbase_atomic.h"
//#include "libbase_log.h"

#include "libbase_ring.h"

#define WNS_RING_NAMESIZE 32 /**< The maximum length of a ring name. */




/**
 * An lock-free ring structure.
 *
 * The producer and the consumer have a head and a tail index. The particularity
 * of these index is that they are not between 0 and size(ring). These indexes
 * are between 0 and 2^32, and we mask their value when we access the ring[]
 * field. Thanks to this assumption, we can do subtractions between 2 index
 * values in a modulo-32bit base: that's why the overflow of the indexes is not
 * a problem.
 */
struct h_ring {
    struct list_head list;      /**< Next in list. */

    char name[WNS_RING_NAMESIZE];    /**< Name of the ring. */
    int32_t flags;                     /**< Flags supplied at creation. */

    /** Ring producer status. */
    struct prod {
        volatile uint32_t bulk_default; /**< Default bulk count. */
        uint32_t watermark;      /**< Maximum items before EDQUOT. */
        uint32_t sp_enqueue;     /**< True, if single producer. */
        uint32_t use_nop;        /**< True, if use null loop for wait, either use yield. */
        uint32_t size;           /**< Size of ring. */
        uint32_t mask;           /**< Mask (size-1) of ring. */
        volatile uint32_t head;  /**< Producer head. */
        volatile uint32_t tail;  /**< Producer tail. */
    } prod __wns_arch_cache_aligned;

    /** Ring consumer status. */
    struct cons {
        volatile uint32_t bulk_default; /**< Default bulk count. */
        uint32_t sc_dequeue;     /**< True, if single consumer. */
        uint32_t use_nop;        /**< True, if use null loop for wait, either use yield. */
        uint32_t size;           /**< Size of the ring. */
        uint32_t mask;           /**< Mask (size-1) of ring. */
        volatile uint32_t head;  /**< Consumer head. */
        volatile uint32_t tail;  /**< Consumer tail. */
    } cons __wns_arch_cache_aligned;

    void * volatile ring[0] \
            __wns_arch_cache_aligned; /**< Memory space of ring starts here. */
};




/**
 * Create a new ring named *name* in memory.
 *
 * This function uses ``memzone_reserve()`` to allocate memory. Its size is
 * set to *count*, which must be a power of two. Water marking is
 * disabled by default. The default bulk count is initialized to 1.
 * Note that the real usable ring size is *count-1* instead of
 * *count*.
 *
 * @param name
 *   The name of the ring.
 * @param count
 *   The size of the ring (must be a power of 2).
 * @param socket_id
 *   The *socket_id* argument is the socket identifier in case of
 *   NUMA. The value can be *SOCKET_ID_ANY* if there is no NUMA
 *   constraint for the reserved zone.
 * @param flags
 *   An OR of the following:
 *    - WNS_RING_F_SP_EN: If this flag is set, the default behavior when
 *      using ``wns_ring_enqueue()`` or ``wns_ring_enqueue_bulk()``
 *      is "single-producer". Otherwise, it is "multi-producers".
 *    - WNS_RING_F_SC_EN: If this flag is set, the default behavior when
 *      using ``wns_ring_dequeue()`` or ``wns_ring_dequeue_bulk()``
 *      is "single-consumer". Otherwise, it is "multi-consumers".
 * @return
 *   On success, the pointer to the new allocated ring. NULL on error with
 *    rte_errno set appropriately. Possible errno values include:
 *    - E_RTE_NO_CONFIG - function could not get pointer to rte_config structure
 *    - E_RTE_SECONDARY - function was called from a secondary process instance
 *    - E_RTE_NO_TAILQ - no tailq list could be got for the ring list
 *    - EINVAL - count provided is not a power of 2
 *    - ENOSPC - the maximum number of memzones has already been allocated
 *    - EEXIST - a memzone with the same name already exists
 *    - ENOMEM - no appropriate memory area found in which to create memzone
 */
h_ring_st *h_ring_create(const char *name, uint32_t count,uint32_t flags);


/**
 * Set the default bulk count for enqueue/dequeue.
 *
 * The parameter *count* is the default number of bulk elements to
 * get/put when using ``wns_ring_*_{en,de}queue_bulk()``. It must be
 * greater than 0 and less than half of the ring size.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param count
 *   A new water mark value.
 * @return
 *   - 0: Success; default_bulk_count changed.
 *   - -EINVAL: Invalid count value.
 */
static inline int32_t
wns_ring_set_bulk_count(h_ring_st *r, uint32_t count)
{
    if (unlikely(count == 0 || count >= r->prod.size))
        return -1;

    r->prod.bulk_default = count;
    r->cons.bulk_default = count;
    return 0;
}

/**
 * Get the default bulk count for enqueue/dequeue.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The default bulk count for enqueue/dequeue.
 */
static inline uint32_t
wns_ring_get_bulk_count(h_ring_st *r)
{
    return r->prod.bulk_default;
}

/**
 * Change the high water mark.
 *
 * If *count* is 0, water marking is disabled. Otherwise, it is set to the
 * *count* value. The *count* value must be greater than 0 and less
 * than the ring size.
 *
 * This function can be called at any time (not necessarilly at
 * initialization).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param count
 *   The new water mark value.
 * @return
 *   - 0: Success; water mark changed.
 *   - -EINVAL: Invalid water mark value.
 */
int32_t h_ring_set_water_mark(h_ring_st *r, uint32_t count);



/**
 * Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table. The
 *   value must be strictly positive.
 * @return
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 */
static inline int32_t
wns_ring_mp_enqueue_bulk(h_ring_st *r, void * const *obj_table,
        uint32_t n)
{
    uint32_t prod_head, prod_next;
    uint32_t cons_tail, free_entries;
    int32_t success;
    uint32_t i;
    uint32_t mask = r->prod.mask;
    int32_t ret;

    /* move prod.head atomically */
    do {
        prod_head = r->prod.head;
        cons_tail = r->cons.tail;
        /* The subtraction is done between two uint32_t 32bits value
         * (the result is always modulo 32 bits even if we have
         * prod_head > cons_tail). So 'free_entries' is always between 0
         * and size(ring)-1. */
        free_entries = (mask + cons_tail - prod_head);

        /* check that we have enough room in ring */
        if (unlikely(n > free_entries)) {
            return -1;
        }

        prod_next = prod_head + n;
        success = wns_atomic32_cmpset(&r->prod.head, prod_head, prod_next);
    } while (unlikely(success == 0));

    /* write entries in ring */
    for (i = 0; likely(i < n); i++)
        r->ring[(prod_head + i) & mask] = obj_table[i];
    wns_wmb();

    /* return -EDQUOT if we exceed the watermark */
    if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark)) {
        ret = -1;
    } else {
        ret = 0;
    }

    /*
     * If there are other enqueues in progress that preceeded us,
     * we need to wait for them to complete
     */
    if (r->prod.use_nop) {
        while (unlikely(r->prod.tail != prod_head))
            wns_nop();
    } else {
        while (unlikely(r->prod.tail != prod_head))
            wns_yield();
    }

    r->prod.tail = prod_next;
    return ret;
}

/**
 * Enqueue several objects on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table. The
 *   value must be strictly positive.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int32_t
wns_ring_sp_enqueue_bulk(h_ring_st *r, void * const *obj_table, uint32_t n)
{
    uint32_t prod_head, cons_tail;
    uint32_t prod_next, free_entries;
    uint32_t i;
    uint32_t mask = r->prod.mask;
    int32_t ret;

    prod_head = r->prod.head;
    cons_tail = r->cons.tail;
    /* The subtraction is done between two uint32_t 32bits value
     * (the result is always modulo 32 bits even if we have
     * prod_head > cons_tail). So 'free_entries' is always between 0
     * and size(ring)-1. */
    free_entries = mask + cons_tail - prod_head;

    /* check that we have enough room in ring */
    if (unlikely(n > free_entries)) {
        return -1;
    }

    prod_next = prod_head + n;
    r->prod.head = prod_next;

    /* write entries in ring */
    for (i = 0; likely(i < n); i++)
        r->ring[(prod_head + i) & mask] = obj_table[i];
    wns_wmb();

    /* return -EDQUOT if we exceed the watermark */
    if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark)) {
        ret = -1;
    }
    else {
        ret = 0;
    }

    r->prod.tail = prod_next;
    return ret;
}

/**
 * Enqueue several objects on a ring.
 *
 * This function calls the multi-producer or the single-producer
 * version depending on the default behavior that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
int32_t
h_ring_enqueue_bulk(h_ring_st *r, void * const *obj_table, uint32_t n)
{
    if (r->prod.sp_enqueue)
        return wns_ring_sp_enqueue_bulk(r, obj_table, n);
    else
        return wns_ring_mp_enqueue_bulk(r, obj_table, n);
}

/**
 * Enqueue one object on a ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int32_t
wns_ring_mp_enqueue(h_ring_st *r, void *obj)
{
    return wns_ring_mp_enqueue_bulk(r, &obj, 1);
}

/**
 * Enqueue one object on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int32_t
wns_ring_sp_enqueue(h_ring_st *r, void *obj)
{
    return wns_ring_sp_enqueue_bulk(r, &obj, 1);
}

/**
 * Enqueue one object on a ring.
 *
 * This function calls the multi-producer or the single-producer
 * version, depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
int32_t
h_ring_enqueue(h_ring_st *r, void *obj)
{
    if (r->prod.sp_enqueue)
        return wns_ring_sp_enqueue(r, obj);
    else
        return wns_ring_mp_enqueue(r, obj);
}

/**
 * Dequeue several objects from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table,
 *   must be strictly positive
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int32_t
wns_ring_mc_dequeue_bulk(h_ring_st *r, void **obj_table, uint32_t n)
{
    uint32_t cons_head, prod_tail;
    uint32_t cons_next, entries;
    int32_t success;
    uint32_t i;
    uint32_t mask = r->prod.mask;

    /* move cons.head atomically */
    do {
        cons_head = r->cons.head;
        prod_tail = r->prod.tail;
        /* The subtraction is done between two uint32_t 32bits value
         * (the result is always modulo 32 bits even if we have
         * cons_head > prod_tail). So 'entries' is always between 0
         * and size(ring)-1. */
        entries = (prod_tail - cons_head);

        /* check that we have enough entries in ring */
        if (unlikely(n > entries)) {
            return -1;
        }

        cons_next = cons_head + n;
        success = wns_atomic32_cmpset(&r->cons.head, cons_head, cons_next);
    } while (unlikely(success == 0));

    /* copy in table */
    wns_rmb();
    for (i = 0; likely(i < n); i++) {
        obj_table[i] = r->ring[(cons_head + i) & mask];
    }

    /*
     * If there are other dequeues in progress that preceeded us,
     * we need to wait for them to complete
     */
    if (r->cons.use_nop) {
        while (unlikely(r->cons.tail != cons_head))
            wns_nop();
    } else {
        while (unlikely(r->cons.tail != cons_head))
            wns_yield();
    }

    r->cons.tail = cons_next;
    return 0;
}

/**
 * Dequeue several objects from a ring (NOT multi-consumers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table,
 *   must be strictly positive.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static int32_t
wns_ring_sc_dequeue_bulk(h_ring_st *r, void **obj_table, uint32_t n)
{
    uint32_t cons_head, prod_tail;
    uint32_t cons_next, entries;
    int32_t i;
    uint32_t mask = r->prod.mask;

    cons_head = r->cons.head;
    prod_tail = r->prod.tail;
    /* The subtraction is done between two uint32_t 32bits value
     * (the result is always modulo 32 bits even if we have
     * cons_head > prod_tail). So 'entries' is always between 0
     * and size(ring)-1. */
    entries = prod_tail - cons_head;

    /* check that we have enough entries in ring */
    if (unlikely(n > entries)) {
        return -1;
    }

    cons_next = cons_head + n;
    r->cons.head = cons_next;

    /* copy in table */
    wns_rmb();
    for (i = 0; likely(i < n); i++) {
        obj_table[i] = r->ring[(cons_head + i) & mask];
    }

    r->cons.tail = cons_next;
    return 0;
}

/**
 * Dequeue several objects from a ring.
 *
 * This function calls the multi-consumers or the single-consumer
 * version, depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
int32_t
h_ring_dequeue_bulk(h_ring_st *r, void **obj_table, uint32_t n)
{
    if (r->cons.sc_dequeue)
        return wns_ring_sc_dequeue_bulk(r, obj_table, n);
    else
        return wns_ring_mc_dequeue_bulk(r, obj_table, n);
}

/**
 * Dequeue one object from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static  int32_t
wns_ring_mc_dequeue(h_ring_st *r, void **obj_p)
{
    return wns_ring_mc_dequeue_bulk(r, obj_p, 1);
}

/**
 * Dequeue one object from a ring (NOT multi-consumers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
static   int32_t
wns_ring_sc_dequeue(h_ring_st *r, void **obj_p)
{
    return wns_ring_sc_dequeue_bulk(r, obj_p, 1);
}

/**
 * Dequeue one object from a ring.
 *
 * This function calls the multi-consumers or the single-consumer
 * version depending on the default behaviour that was specified at
 * ring creation time (see flags).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success, objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue, no object is
 *     dequeued.
 */
int32_t
h_ring_dequeue(h_ring_st *r, void **obj_p)
{
    if (r->cons.sc_dequeue)
        return wns_ring_sc_dequeue(r, obj_p);
    else
        return wns_ring_mc_dequeue(r, obj_p);
}

/**
 * Test if a ring is full.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   - 1: The ring is full.
 *   - 0: The ring is not full.
 */
int32_t
h_ring_full(const h_ring_st *r)
{
    uint32_t prod_tail = r->prod.tail;
    uint32_t cons_tail = r->cons.tail;
    return (((cons_tail - prod_tail - 1) & r->prod.mask) == 0);
}

/**
 * Test if a ring is empty.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   - 1: The ring is empty.
 *   - 0: The ring is not empty.
 */
int32_t
h_ring_empty(const h_ring_st *r)
{
    uint32_t prod_tail = r->prod.tail;
    uint32_t cons_tail = r->cons.tail;
    return !!(cons_tail == prod_tail);
}

/**
 * Return the number of entries in a ring.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The number of entries in the ring.
 */
uint32_t
h_ring_count(const h_ring_st *r)
{
    uint32_t prod_tail = r->prod.tail;
    uint32_t cons_tail = r->cons.tail;
    return ((prod_tail - cons_tail) & r->prod.mask);
}

/**
 * Return the number of free entries in a ring.
 *
 * @param r
 *   A pointer to the ring structure.
 * @return
 *   The number of free entries in the ring.
 */
uint32_t
h_ring_free_count(const h_ring_st *r)
{
    uint32_t prod_tail = r->prod.tail;
    uint32_t cons_tail = r->cons.tail;
    return ((cons_tail - prod_tail - 1) & r->prod.mask);
}


/**
 * Search a ring from its name
 *
 * @param name
 *   The name of the ring.
 * @return
 *   The pointer to the ring matching the name, or NULL if not found,
 *   with rte_errno set appropriately. Possible rte_errno values include:
 *    - ENOENT - required entry not available to return.
 */
h_ring_st *wns_ring_lookup(const char *name);


/* global list of ring (used for debug/dump) */
//static struct list_head ring_list = {NULL, NULL};
//debug dump的东西稍后实现，现在放在这里不能线程安全

/* create the ring */
h_ring_st *
h_ring_create(const char *name, uint32_t count, uint32_t flags)
{
    h_ring_st *r = NULL;
    uint32_t ring_size = 0;

    if (count == 0)
        return NULL;
    /* count must be a power of 2 */
    if (!POWEROF2(count)) {
        printf("Requested size is not a power of 2\n");
        return NULL;
    }

    ring_size = count * sizeof(void *) + sizeof(h_ring_st);

    /* reserve a memory zone for this ring. If we can't get rte_config or
     * we are secondary process, the memzone_reserve function will set
     * rte_errno for us appropriately - hence no check in this this function */
    r = (h_ring_st *)h_malloc(ring_size);
    if (r == NULL) {
        printf("Cannot alloc memory\n");
        return NULL;
    }

    /* init the ring structure */
    memset(r, 0, sizeof(*r));
    sprintf_n(r->name, WNS_RING_NAMESIZE, "%s", name);
    r->flags = flags;
    r->prod.bulk_default = 1;
    r->cons.bulk_default = 1;
    r->prod.watermark = count;
    r->prod.sp_enqueue = !!(flags & WNS_RING_F_SP_EN);
    r->prod.use_nop = !!(flags & WNS_RING_F_MORE_CORE);
    r->cons.sc_dequeue = !!(flags & WNS_RING_F_SC_EN);
    r->cons.use_nop = !!(flags & WNS_RING_F_MORE_CORE);
    r->prod.size = count;
    r->cons.size = count;
    r->prod.mask = count -1;
    r->cons.mask = count-1;
    r->prod.head = 0;
    r->cons.head = 0;
    r->prod.tail = 0;
    r->cons.tail = 0;

    //list_add_tail(&r->list, &ring_list);
    return r;
}

void h_ring_destroy(h_ring_st *ring)
{
    h_free(ring);
}

/*
 * change the high water mark. If *count* is 0, water marking is
 * disabled
 */
int32_t
h_ring_set_water_mark(h_ring_st *r, uint32_t count)
{
    if (count >= r->prod.size)
        return -1;

    /* if count is 0, disable the watermarking */
    if (count == 0)
        count = r->prod.size;

    r->prod.watermark = count;
    return 0;
}


/* search a ring from its name */
#if 0
wns_ring_st *
wns_ring_lookup(const char *name)
{
    wns_ring_st *r=NULL;
    struct list_head *phead=NULL;

    list_for_each(phead, &ring_list)
    {
        if(strncmp(name, r->name, WNS_RING_NAMESIZE) == 0)
        {
            r = list_entry(phead, struct wns_ring, list);
            break;
        }
    }

    return r;
}

#endif
