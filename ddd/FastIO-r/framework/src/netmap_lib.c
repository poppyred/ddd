//	$Id: netmap_lib.c 2013-05-23 likunxiang$
//
/*
 * (C) 2011-2012 Luigi Rizzo
 *
 * BSD license
 *
 * A simple library that maps some pcap functions onto netmap
 * This is not 100% complete but enough to let tcpdump, trafshow
 * and other apps work.
 *
 * $FreeBSD: head/tools/tools/netmap/pcap.c 227614 2011-11-17 12:17:39Z luigi $
 */

#include <sys/syscall.h>
#include <assert.h>
#include "fio_nm_util.h"
#include "netmap_lib.h"
#include "fio_lfqueue.h"
#define gettid() syscall(__NR_gettid) 

uint64_t g_cap_num = 0;
int g_snd_num = 0;
int do_shutdown = 0;
int g_bigend = 0;
struct ef_ring *g_me;
int g_thd_num = 1;
LFQueue * g_msg_queues[MAX_THD_NUM];
pthread_t g_work_thdid[MAX_THD_NUM];

static fun_proc g_deal_funs[MAX_THD_NUM];

#ifdef ADD_COPY_LOCK
#define COPY_LOCK(q) while (__sync_lock_test_and_set(&(q),1)) {}
#define COPY_UNLOCK(q) __sync_lock_release(&(q))
#endif

#define EF_MAX_QUEUE 128

static int g_num_copied[EF_MAX_QUEUE]; //each ringid
#ifdef ADD_COPY_LOCK
static int g_lock_num_copied[EF_MAX_QUEUE]; //each ringid
#endif
static u_int g_id_puts[EF_MAX_QUEUE]; //each ringid
static int * g_copy_flags[EF_MAX_QUEUE]; //each ringid
static int * g_copy_head[EF_MAX_QUEUE]; //each ringid

static uint32_t g_recv_pkts[EF_MAX_QUEUE]; //each ringid

struct ef_ring {
    struct my_ring me;
    int snaplen;
    int promisc;
};

int inc_num_copied(u_int r_id, u_int c)
{
#ifdef ADD_COPY_LOCK_1
    COPY_LOCK(g_lock_num_copied[r_id]);
#endif
    g_num_copied[r_id] += c;
#ifdef ADD_COPY_LOCK_1
    COPY_UNLOCK(g_lock_num_copied[r_id]);
#endif
    return 0;
}

int inc_num_copied2(u_int r_id, u_int s, u_int e, u_int expect_len)
{
    int *cf = g_copy_flags[r_id];
    if (!cf)
        return -1;

    const u_int rx_len = g_me->me.nr_rx_slots;
#ifdef ADD_COPY_LOCK
    int s1;
    int *p = cf+e+1;
    int * const pe = cf+rx_len-1;
#endif

#ifdef ADD_COPY_LOCK
    int s1;
    COPY_LOCK(g_lock_num_copied[r_id]);
    int *h = g_copy_head[r_id];
#endif
    if (s <= e)
        cf[s] = e-s+1;
    else
        cf[s] = rx_len-(s-e-1);

#ifdef ADD_COPY_LOCK
    if (cf[s] != expect_len)
    {
        D("<error> si %u expect_len %u != %u s %u e %u rx_len %u", 
                r_id, expect_len, cf[s], s, e, rx_len);
        assert(0);
    }

    ND("si %u h %lu s %u e %u", r_id, h-cf, s, e);
    while (p != h)
    {
        s1 = *p;
        if (s1 == 0)
            break;
        cf[s] += s1;
        *p = 0;
        if (p + s1 > pe) 
            p = p+s1-rx_len;
        else
            p += s1;
        ND("si %u h %lu p %lu", r_id, h-cf, p-cf);
    }
    COPY_UNLOCK(g_lock_num_copied[r_id]);
#endif

    return 0;
}

int checkCPUendian()
{
    union{
        unsigned long int i;
        unsigned char s[4];
    }c = {0x12345678};

    return (0x12 == c.s[0]);
}

inline void init_trans(struct traninfo *trans, int size)
{
    int pos_tran;
    for (pos_tran = 0; pos_tran < size; pos_tran++)
    {
        trans[pos_tran].size = 0;
        trans[pos_tran].ring_id = -1;
        trans[pos_tran].buf = NULL;
    }
}

#if 0
#define _WHICH_ 1
static void * working(void *data)
{
    int myid= *(int*)data;
    LFQueue *q_msg = g_msg_queues[myid];
    D("threadid %lu, posid %d", gettid(), myid);

    char data_buf[10000];
    struct traninfo tran[MAX_PKT_COPY_ONCE];
    u_int prev_rid, proc_pkt, _s, _e;
    int got, pos;        
    while(!do_shutdown)
    {
        if ((got=LFQ_try_get2(q_msg, tran, MAX_PKT_COPY_ONCE)) < 1)
        {
            usleep(50);
            continue;
        }

        ND("threadid %d got %d pkt", myid, got);
        prev_rid = tran[0].ring_id;
        _s = _e = tran[0].slot_id;
        proc_pkt = 0;
        for (pos = 0; pos < got; pos++)
        {
#if (_WHICH_==1)
            if (prev_rid != tran[pos].ring_id)
            {
                //change g_num_copied
                //inc_num_copied(prev_rid, proc_pkt);
                inc_num_copied2(prev_rid, _s, _e, proc_pkt);
                prev_rid = tran[pos].ring_id;
                proc_pkt = 0;
                _s = _e = tran[pos].slot_id;
            }
            else
                _e = tran[pos].slot_id;
#endif

#if (_WHICH_==0)
            inc_num_copied(tran[pos].ring_id, 1);
#endif
            deal(myid, tran+pos, data_buf);
#if (_WHICH_==1)
            proc_pkt++;
#endif
        }

#if (_WHICH_==1)
        if (proc_pkt > 0)
        {

            inc_num_copied2(prev_rid, _s, _e, proc_pkt);
        }
#endif
    }
    return NULL;
}
#endif

static void * routine(void *data)
{
    int myid= *(int*)data;
    LFQueue * const q_msg = g_msg_queues[myid];
    D("threadid %lu, posid %d", gettid(), myid);

    fun_proc deal = g_deal_funs[myid];

    char data_buf[10000];
    struct traninfo tran[MAX_PKT_COPY_ONCE];
    struct traninfo temp_tran;
    u_int proc_pkt, _s = 0, _e = 0, si = 0;
    int got, pos, got_once, _list_len = 0;        
    int need_set_s;
loop_tag:
    while(!do_shutdown)
    {
        if (LFQ_top(q_msg, &temp_tran, sizeof(struct traninfo)) != 0)
        {
            usleep(10);
            continue;
        }

        _list_len = temp_tran.list_len;
        got = 0;
        got_once = 0;
        while (got < MAX_PKT_COPY_ONCE)
        {
            if ((got_once=LFQ_try_get2(q_msg, tran+got, _list_len)) < 1)
            {
                D("threadid %d got nothing list_len %d rid %u", myid, _list_len, temp_tran.ring_id);
                usleep(10);
                goto loop_tag;
            }

            if (got_once > _list_len)
            {
                D("<error> threadid %d got_once %d > list_len %d", myid, got_once, _list_len);
                assert(got_once <= _list_len);
                //exit(1);
            }

            got += got_once;

            if (LFQ_top(q_msg, &temp_tran, sizeof(struct traninfo)) != 0)
                break;

            _list_len = temp_tran.list_len;
	    ND("threadid %d l %d rid %u", myid, _list_len, temp_tran.ring_id);
            if (_list_len + got > MAX_PKT_COPY_ONCE)
                break;
        }

        ND("threadid %d got %d pkt", myid, got);
        if (0 == (rf_s&tran[0].range))
        {
            D("threadid %d first tran is not the beginning tran! slot id: %u", 
                    myid, tran[0].slot_id);
            assert(0);
        }

        proc_pkt = 0;
        need_set_s = 1;
        for (pos = 0; pos < got; pos++)
        {
            if (need_set_s)
            {
                _s = _e = tran[pos].slot_id;
                need_set_s = 0;
            }

            deal(myid, tran+pos, data_buf);
            proc_pkt++;
            si = tran[pos].ring_id;

            //_e = tran[pos].slot_id;	//请注释

            if (rf_e&tran[pos].range)
            {
                _e = tran[pos].slot_id;
                inc_num_copied2(tran[pos].ring_id, _s, _e, proc_pkt);
                proc_pkt = 0;
                need_set_s = 1;
            }
        }

        if (proc_pkt > 0)
        {//error
            D("threadid %d the last tran is not the ending tran! si %u s %u e %u got %d", 
                    myid, si, _s, tran[pos-1].slot_id, got);
            assert(proc_pkt<=0);
            //exit(1);
        }
    }
    return NULL;
}

int init_msg_queues(int q_size, fun_proc fun)
{
    static int pos_array[MAX_THD_NUM];
    int pos;
    for (pos = 0; pos < g_thd_num; pos++)
    {
        if (NULL == (g_msg_queues[pos]=LFQ_create(sizeof(struct traninfo), q_size)))
        {
            D("create msgqueue failed, pos %d", pos);
            return -1;
        }
    }

    for (pos = 0; pos < g_thd_num; pos++)
    {
        pos_array[pos] = pos;
        g_deal_funs[pos] = fun;
        pthread_create(g_work_thdid+pos, NULL, routine, pos_array+pos);

    }

    return 0;
}

int destroy_msg_queues()
{
    int pos;
    for (pos = 0; pos < g_thd_num; pos++)
        LFQ_destroy(g_msg_queues[pos]);
    return 0;
}

inline int ef_do_filter(u_int16_t port, const u_char *buf/*, size_t size*/)
{
    if (!port)
        return 0;
    if ( port == *(u_short*)&buf[36])
        return 0;

    return -1;
}

char * ef_lookupdev(char *buf)
{
    D("%s", buf);
    strcpy(buf, "/dev/netmap");
    return buf;
}

int ef_bind(const char *device, int port)
{
    int promisc = 1;
    int l;

    if (!device) {
        D("missing device name");
        return -1;
    }

    l = strlen(device) + 1;
    D("request to open %s promisc %d", device, promisc);

    g_me = calloc(1, sizeof(*g_me) + l);
    if (g_me == NULL) 
    {
        D("failed to allocate struct for %s", device);
        return -1;
    }
    g_me->me.ifname = (char *)(g_me + 1);
    strcpy((char *)g_me->me.ifname, device);
    if (netmap_open(&g_me->me, 0, promisc)) 
    {
        D("error opening %s", device);
        free(g_me);
        return -1;
    }
    g_me->me.port = (checkCPUendian() ? port : BigLittleSwap16(port));

    for (l = 0; l < g_me->me.nr_rx_rings; l++)
    { 
        g_copy_flags[l] = calloc(g_me->me.nr_rx_slots, sizeof(int));
        g_copy_head[l] = g_copy_flags[l];
    }

    return g_me->me.fd;
}

void ef_close(int fd)
{
    (void)fd;
    //struct my_ring *me;
    struct my_ring *me = (struct my_ring*)g_me;
    int l;

    D("");

    for (l = 0; l < me->nr_rx_rings; l++)
    { 
        free(g_copy_flags[l]);
        g_copy_flags[l] = NULL;
    }

    if (me->mem)
        munmap(me->mem, me->memsize);
    /* restore original flags ? */
    ioctl(me->fd, NIOCUNREGIF, NULL);
    close(me->fd);
    bzero(me, sizeof(*me));
    free(me);
}

//return: the number of packet received
int ef_recvmsg(int fd, struct traninfo *trans, uint32_t size)
{
    (void)fd;
    //struct ef_ring *pme;
    struct ef_ring *pme = g_me;
    struct my_ring *me;;
    struct traninfo *prev_tran = trans;
    uint32_t got = 0;
    u_int si;

    me = &pme->me;

    /* scan all rings */
    for (si = me->the_begin; si < me->the_end && got < size; si++) {
        struct netmap_ring *ring = NETMAP_RXRING(me->nifp, si);
        //ND("ring has %d pkts", ring->avail);
        if (ring->avail == 0)
            continue;
        //pme->hdr.ts = ring->ts;

        u_int i_slot, idx;
        u_char *rbuf;
        //int recv_len = 0;
        struct netmap_slot *slot;
        struct traninfo *temp_tran = prev_tran;
        uint32_t got_once = 0;
        //D("%u", ring->avail);
        while (got_once < ring->avail) 
        {
            i_slot = ring->cur;
            slot = &ring->slot[i_slot];
            idx = slot->buf_idx;
            if (idx < 2) 
            {
                D("%s bogus RX index %d at offset %d", me->nifp->ni_name, idx, i_slot);
                sleep(2);
            }
            rbuf = (u_char *)NETMAP_BUF(ring, idx);
            if (slot->len > 41 && !ef_do_filter(me->port, rbuf))
                //if (slot->len > 41 && (me->port == 0 || me->port == *(u_int16_t*)&rbuf[36]))
                //if (0 == ef_do_filter(me, rbuf, slot->len))
            {
                //prefetch(rbuf);
                //*temp_tran->head.p_addr = *(u_long*)(rbuf + g_iphdr_offset);
                //*temp_tran->head.p_port = *(u_short*)(rbuf + g_sport_offset);
                //temp_tran->head.size = recv_len = slot->len-42;
                //if (recv_len > MAX_PAYLOAD_SIZE)
                //    temp_tran->head.size = recv_len = MAX_PAYLOAD_SIZE;
                ////D("%d %u %u", slot->len, got_once, ring->avail);
                //memcpy(temp_tran->buf, rbuf+42, recv_len);
                temp_tran++;
                //ND("equal");
            }
            ring->cur = NETMAP_RING_NEXT(ring, i_slot);
            got_once++;
        }
        ring->avail -= got_once;
        got += got_once;
        prev_tran = temp_tran;
    }
    g_cap_num += got;
    return got;
}

#if 0
static int rid_qpos[EF_MAX_QUEUE];
    __attribute__((constructor))
static void rid_qpos_init(void)
{
    int i;
    for (i = 0; i < EF_MAX_QUEUE; i++)
        rid_qpos[i] = -1;
}
#endif

int ef_flush_rx_queue(u_int r_id, struct traninfo *trans, int num_buf)
{
    static int q_pos = 0;
    LFQueue *q_msg = NULL;

#if 0
    int q_id;
    if ((q_id=rid_qpos[r_id]) == -1)
    {
        rid_qpos[r_id] = q_id = q_pos++;
        q_msg = g_msg_queues[q_id];
        if (q_pos >= g_thd_num)
            q_pos = 0;
    }
    else
    {
        q_msg = g_msg_queues[q_id];
    }
#else
    q_msg = g_msg_queues[q_pos];
    ND("si %u qid %d num_buf %u, s %u e %u", r_id, q_pos, num_buf,
            trans[0].slot_id, trans[num_buf-1].slot_id);
    trans[0].range |= rf_s;
    trans[0].list_len = num_buf;
    trans[num_buf-1].range |= rf_e;
#endif
    if (0 != LFQ_try_put4(q_pos, q_msg, trans, num_buf))
    {
        D("WARN!! %d queue is full", q_pos);
        return -1;
    }


    //static int v_abc = 0;
    //v_abc += num_buf;
    //D("r_id %u, q_id %d num %d!", r_id, q_id, v_abc);
    ND("si %u, qid %d num %d!", r_id, q_pos, num_buf);

    if (++q_pos >= g_thd_num)
        q_pos = 0;

    return 0;
}

static struct traninfo arr_trans[MAX_PKT_COPY_ONCE];
    __attribute__((constructor))
static void pkt_buf_init(void)
{
    if (checkCPUendian())
        g_bigend = 1;
    init_trans(arr_trans, MAX_PKT_COPY_ONCE);
}

int ef_mainrecvmsg(int fd)
{
    (void)fd;
    u_int si;
    struct ef_ring *pme = g_me;
    struct my_ring *me;;
    uint32_t got = 0;
    struct netmap_ring *ring;
    struct traninfo *t_tran = arr_trans;
    int num_buf = 0;

    me = &pme->me;

    const u_int rx_slots = me->nr_rx_slots;
    for (si = me->the_begin; si < me->the_end; si++) 
    {
        ring = NETMAP_RXRING(me->nifp, si);
        if (ring->avail == 0 || ring->avail < ring->num_delay_copys || ring->avail - ring->num_delay_copys == 0)
            continue;

        u_int i_slot, idx, id_put = g_id_puts[si];
        u_char *rbuf;
        struct netmap_slot *slot;
        uint32_t got_once = 0, avail, num_delay;
        uint32_t igr_slot = 0;

        if (ring->avail < ring->num_delay_copys)
        {
            D("<error> si %u ring->avail %u ring->num_delay_copys %u", si, ring->avail, ring->num_delay_copys);
            return -1;
        }
        i_slot = ring->cur;
        num_delay = ring->num_delay_copys;
        avail = ring->avail - num_delay;

        while (igr_slot < num_delay)
        {
            i_slot = NETMAP_RING_NEXT(ring, i_slot);
            igr_slot++;
        }

        while (got_once < avail) 
        {

            slot = &ring->slot[i_slot];
            idx = slot->buf_idx;
            if (idx < 2) 
            {
                D("%s bogus RX index %d at offset %d", me->nifp->ni_name, idx, i_slot);
                sleep(2);
            }
            rbuf = (u_char *)NETMAP_BUF(ring, idx);
            //if (slot->len > 41 && !ef_do_filter(me->port, rbuf))
            //{
            t_tran->buf = NULL; 
            t_tran->ring_id = si; 
            t_tran->slot_id = (id_put++%rx_slots); 
            t_tran->range = rf_n; 
            t_tran->list_len = 0;
            t_tran->size = (slot->len > MAX_PAYLOAD_SIZE ? MAX_PAYLOAD_SIZE : slot->len);
            ND("rid %d size %d", t_tran->ring_id, t_tran->size);

            if (t_tran->size > 0)
            {
                t_tran->buf = rbuf;
            }

            num_buf++;
            t_tran++;

            if (num_buf == MAX_PKT_COPY_ONCE)
            {//flush into queue
                D("si %u num_buf == Max, s %u e %u", si,
                        arr_trans[0].slot_id, arr_trans[num_buf-1].slot_id);
                ef_flush_rx_queue(si, arr_trans, num_buf);
                ring->num_delay_copys += num_buf;
                num_buf = 0;
                t_tran = arr_trans;
            }
            //}
            //slot->flags |= NS_DELAY_COPY;
            ND("si %u num_delay_copys %u", si, ring->num_delay_copys);
            i_slot = NETMAP_RING_NEXT(ring, i_slot);
            got_once++;
        }

        if (num_buf > 0)
        {//flush into queue
            //D("si %u num_buf %u, s %u e %u", si, num_buf,
            //		arr_trans[0].slot_id, arr_trans[num_buf-1].slot_id);
            ef_flush_rx_queue(si, arr_trans, num_buf);
            ring->num_delay_copys += num_buf;
            num_buf = 0;
            t_tran = arr_trans;
        }

        g_id_puts[si] = id_put%rx_slots;

        got += got_once;
        //g_recv_pkts[si] += got_once;
    }

    //if (num_buf > 0)
    //{//flush into queue
    //    ef_flush_rx_queue(si, arr_trans, num_buf);
    //}

    //int num_copy;
    //for (si = me->the_begin; si < me->the_end; si++) 
    //{
    //    ring = NETMAP_RXRING(me->nifp, si);
    //    if (ring->num_delay_copys == 0)
    //        continue;

    //    COPY_LOCK(g_lock_num_copied[si]);
    //    num_copy = g_num_copied[si];
    //    if (num_copy != 0) 
    //        g_num_copied[si] = 0;
    //    COPY_UNLOCK(g_lock_num_copied[si]);

    //    if (num_copy <= 0)
    //        continue;

    //    uint32_t got_once = 0;
    //    while (got_once < num_copy) 
    //    {
    //        ring->cur = NETMAP_RING_NEXT(ring, ring->cur);
    //        got_once++;
    //    }
    //    ND("si %u ring->avail %u, got_once %u", si, ring->avail, got_once);
    //    ring->avail -= got_once;
    //    ND("after si %u ring->avail %u\n", si, ring->avail);
    //    ring->num_delay_copys -= got_once;
    //}

    g_cap_num += got;
    ND("got %u !!!!!\n", got);
    return got;
}

int ef_sync()
{
    u_int si;
    struct my_ring *me = &g_me->me;
    struct netmap_ring *ring;
    int num_copy;
    for (si = me->the_begin; si < me->the_end; si++) 
    {
        ring = NETMAP_RXRING(me->nifp, si);
        if (ring->num_delay_copys == 0)
            continue;

#ifdef ADD_COPY_LOCK_1
        COPY_LOCK(g_lock_num_copied[si]);
#endif
        num_copy = g_num_copied[si];
        if (num_copy != 0) 
            g_num_copied[si] = 0;
#ifdef ADD_COPY_LOCK_1
        COPY_UNLOCK(g_lock_num_copied[si]);
#endif

        if (num_copy <= 0)
            continue;

        uint32_t got_once = 0;
        while (got_once < num_copy) 
        {
            ring->cur = NETMAP_RING_NEXT(ring, ring->cur);
            got_once++;
        }
        ND("si %u ring->avail %u, got_once %u", si, ring->avail, got_once);
        ring->avail -= got_once;
        ND("after si %u ring->avail %u, num_delay %u", si, ring->avail, ring->num_delay_copys);
        ring->num_delay_copys -= got_once;
        ND("after si %u ring->num_delay_copys %u\n", si, ring->num_delay_copys);
    }

    return 0;
}

int ef_sync2()
{
    u_int si;
    struct my_ring *me = &g_me->me;
    struct netmap_ring *ring;
    const u_int rx_slots = me->nr_rx_slots;
    for (si = me->the_begin; si < me->the_end; si++) 
    {
        ring = NETMAP_RXRING(me->nifp, si);
        if (ring->num_delay_copys == 0)
            continue;

        int num_copy = 0;
        int *cf = g_copy_flags[si];
        int * const pe = cf+rx_slots-1;
        int num_v;
#ifdef ADD_COPY_LOCK
        COPY_LOCK(g_lock_num_copied[si]);
#endif
        int *h = g_copy_head[si];
        while ( (num_v=*h) != 0)
        {
            *h = 0;
            if (h+num_v > pe)
                h = h+num_v-rx_slots;
            else 
                h += num_v;
            num_copy += num_v;
            ND("si %u h %lu num_v %d num_copy %d", si, h-cf, num_v, num_copy);
        }
        g_copy_head[si] = h;
        ND("si %u num_copy %u", si, num_copy);

#ifdef ADD_COPY_LOCK
        COPY_UNLOCK(g_lock_num_copied[si]);
#endif

        if (num_copy <= 0)
            continue;

        uint32_t got_once = 0;
        while (got_once < num_copy) 
        {
            ring->cur = NETMAP_RING_NEXT(ring, ring->cur);
            got_once++;
        }
        ND("si %u got_once %u avail %u delay %u", 
                si, got_once, ring->avail, ring->num_delay_copys);
        ring->avail -= got_once;
        ring->num_delay_copys -= got_once;
        ND("after si %u avail %u num_delay_copys %u\n", 
                si, ring->avail, ring->num_delay_copys);
    }

    return 0;
}

int ef_sendmsg(int sockfd, const void *buff, size_t nbytes, int flags)
{
    (void)sockfd;
    (void)flags;
    //struct my_ring *me;
    struct my_ring *me = (struct my_ring*)g_me;
    u_int si;

    ND("cnt %d", cnt);
    /* scan all rings */
    for (si = me->the_begin; si < me->the_end; si++) {
        struct netmap_ring *ring = NETMAP_TXRING(me->nifp, si);

        ND("ring has %d pkts", ring->avail);
        if (ring->avail == 0)
            continue;
        u_int i = ring->cur;
        u_int idx = ring->slot[i].buf_idx;
        if (idx < 2) {
            D("%s bogus TX index %d at offset %d",
                    me->nifp->ni_name, idx, i);
            sleep(2);
        }
        u_char *dst = (u_char *)NETMAP_BUF(ring, idx);
        ring->slot[i].len = nbytes;
        nm_pkt_copy(buff, dst, nbytes);
        ring->cur = NETMAP_RING_NEXT(ring, i);
        ring->avail--;
        //g_snd_num++;
        // if (ring->avail == 0) ioctl(me->fd, NIOCTXSYNC, NULL);
        return nbytes;
    }
    errno = ENOBUFS;
    return -1;
}

int ef_send(int sockfd, const void *buff, size_t nbytes, int flags)
{
    (void)sockfd;
    (void)flags;
    //struct my_ring *me;
    struct my_ring *me = (struct my_ring*)g_me;
    u_int si;

    ND("cnt %d", cnt);
    /* scan all rings */
    for (si = me->the_begin; si < me->the_end; si++) {
        struct netmap_ring *ring = NETMAP_TXRING(me->nifp, si);

        ND("ring has %d pkts", ring->avail);
        if (ring->avail == 0)
            continue;
        u_int i = ring->cur;
        u_int idx = ring->slot[i].buf_idx;
        if (idx < 2) {
            D("%s bogus TX index %d at offset %d",
                    me->nifp->ni_name, idx, i);
            sleep(2);
        }
        u_char *dst = (u_char *)NETMAP_BUF(ring, idx);
        ring->slot[i].len = nbytes;
        nm_pkt_copy(buff, dst, nbytes);
        ring->cur = NETMAP_RING_NEXT(ring, i);
        ring->avail--;
        //g_snd_num++;
        // if (ring->avail == 0) ioctl(me->fd, NIOCTXSYNC, NULL);
        return nbytes;
    }
    errno = ENOBUFS;
    return -1;
}

void * ef_count_pkt(void *data)
{
    (void)data;
    struct timeval tic, toc; 
    int report_interval = 2000;
    uint64_t prev = 0;

    gettimeofday(&toc, NULL);
    for (;;) {
        struct timeval now, delta;
        uint64_t pps; 
        int pos;
        static char buf_dump[2000];

        delta.tv_sec = report_interval/1000;
        delta.tv_usec = (report_interval%1000)*1000;
        select(0, NULL, NULL, NULL, &delta);
        gettimeofday(&now, NULL);
        timersub(&now, &toc, &toc);
        pps = toc.tv_sec* 1000000 + toc.tv_usec;
        if (pps < 10000)
            continue;
        pps = (g_cap_num - prev) * 1000000 / pps; 
        D("%lu pps, %lu", pps, g_cap_num);
        toc = now; 
        prev = g_cap_num;

        sprintf(buf_dump, "%u", g_recv_pkts[0]);
        for (pos = 1; pos < g_me->me.nr_rx_rings; pos++)
            sprintf(buf_dump, "%s  %u", buf_dump, g_recv_pkts[pos]);
        ND("%s\n", buf_dump);

        if(do_shutdown)
            break;
    }    

    timerclear(&tic);
    timerclear(&toc);

    return NULL;
}

int ef_recvmsgs(int fd)
{
    (void)fd;
    u_int si;
    struct ef_ring *pme = g_me;
    struct my_ring *me;;
    uint32_t got = 0;
    struct netmap_ring *ring;
    struct traninfo *t_tran = arr_trans;
    int num_buf = 0;

    me = &pme->me;

    const u_int rx_slots = me->nr_rx_slots;
    for (si = me->the_begin; si < me->the_end; si++) 
    {
        ring = NETMAP_RXRING(me->nifp, si);
        if (ring->avail == 0 || ring->avail < ring->num_delay_copys || ring->avail - ring->num_delay_copys == 0)
            continue;

        u_int i_slot, idx, id_put = g_id_puts[si];
        u_char *rbuf;
        struct netmap_slot *slot;
        uint32_t got_once = 0, avail, num_delay;
        uint32_t igr_slot = 0;

        if (ring->avail < ring->num_delay_copys)
        {
            D("<error> si %u ring->avail %u ring->num_delay_copys %u", si, ring->avail, ring->num_delay_copys);
            return -1;
        }
        i_slot = ring->cur;
        num_delay = ring->num_delay_copys;
        avail = ring->avail - num_delay;

        while (igr_slot < num_delay)
        {
            i_slot = NETMAP_RING_NEXT(ring, i_slot);
            igr_slot++;
        }

        while (got_once < avail) 
        {

            slot = &ring->slot[i_slot];
            idx = slot->buf_idx;
            if (idx < 2) 
            {
                D("%s bogus RX index %d at offset %d", me->nifp->ni_name, idx, i_slot);
                sleep(2);
            }
            rbuf = (u_char *)NETMAP_BUF(ring, idx);
            //if (slot->len > 41 && !ef_do_filter(me->port, rbuf))
            //{
            t_tran->buf = NULL; 
            t_tran->ring_id = si; 
            t_tran->slot_id = (id_put++%rx_slots); 
            t_tran->range = rf_n; 
            t_tran->list_len = 0;
            t_tran->size = (slot->len > MAX_PAYLOAD_SIZE ? MAX_PAYLOAD_SIZE : slot->len);
            ND("rid %d size %d", t_tran->ring_id, t_tran->size);

            if (t_tran->size > 0)
            {
                t_tran->buf = rbuf;
            }

            num_buf++;
            t_tran++;

            if (num_buf == MAX_PKT_COPY_ONCE)
            {//flush into queue
                D("si %u num_buf == Max, s %u e %u", si,
                        arr_trans[0].slot_id, arr_trans[num_buf-1].slot_id);
                ef_flush_rx_queue(si, arr_trans, num_buf);
                ring->num_delay_copys += num_buf;
                num_buf = 0;
                t_tran = arr_trans;
            }
            //}
            //slot->flags |= NS_DELAY_COPY;
            ND("si %u num_delay_copys %u", si, ring->num_delay_copys);
            i_slot = NETMAP_RING_NEXT(ring, i_slot);
            got_once++;
        }

        if (num_buf > 0)
        {//flush into queue
            //D("si %u num_buf %u, s %u e %u", si, num_buf,
            //		arr_trans[0].slot_id, arr_trans[num_buf-1].slot_id);
            ef_flush_rx_queue(si, arr_trans, num_buf);
            ring->num_delay_copys += num_buf;
            num_buf = 0;
            t_tran = arr_trans;
        }

        g_id_puts[si] = id_put%rx_slots;

        got += got_once;
        //g_recv_pkts[si] += got_once;
    }

    g_cap_num += got;
    ND("got %u !!!!!\n", got);
    return got;
}

