//	$Id: fio_context.c 2013-05-23 likunxiang$
//
#define _GNU_SOURCE
#include <sched.h>
#include <assert.h>
#include "fio_context.h"
#include "fio_route_manager.h"
#include "fio_timer.h"
#include "fio_handler.h"
#include "fio_pack_misc.h"
#include "fio_statistics.h"
#include "fio_sysconfig.h"
#include "fio_map32.h"
#include "fio_mapstr.h"
#include "fio_syslog.h"
#include "fio_sysstack.h"
#include "fio_sockpair.h"
#include "log_util.h"
#include "fio_dns_logic.h"
#include "fio_test_send.h"
#include "log_log.h"

struct fio_context *g_contexts;
struct fio_context *g_syscontext;

static pthread_t g_count_id = 0;
static int g_started_count = 0;

#define LOCKFILE "/tmp/nfio.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
static int lockfile(int fd)
{
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return(fcntl(fd, F_SETLK, &fl));
}
static int already_running(const char *filename)
{
    int fd;
    char buf[16];
    fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
    if (fd < 0) {
        exit(1);
    }
    if (lockfile(fd) == -1) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return 1;
        }
        exit(1);
    }
    int sret = ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    sret = write(fd, buf, strlen(buf) + 1);
    (void)sret;
    return 0;
}
void fio_sigint_h(int sig)
{
    int i;
    static int siged = 0;
    if (siged)
        return;
    siged = 1;
    OD( "sig %d", sig);

    fio_close_timer();

    if (sysconfig.enable_sysstack && g_syscontext && g_syscontext->thread)
    {
        OD( "Cancelling thread sysstack");
        pthread_cancel(g_syscontext->thread);
    }

    if (g_started_count && g_count_id)
    {
        OD( "Cancelling thread count");
        pthread_cancel(g_count_id);
        g_started_count = 0;
    }

    for (i = 0; i < sysconfig.nthreads; i++) 
    {
        /* cancel active threads. */
        if (g_contexts[i].used == 0)
            continue;

        OD( "Cancelling thread #%d\n", i);
        if (g_contexts[i].thread)
            pthread_cancel(g_contexts[i].thread);
        g_contexts[i].used = 0;
    }

    sysconfig.maclog.vtbl.close(&sysconfig.maclog);

    //fio_shutdown();

    //signal(SIGINT, SIG_DFL);
}

static inline void fio_refresh_nictxlimit(struct fio_nic *rn)
{
    rn->txslow = rn->nictxavl - rn->txfast;
    if (rn->txslow < 0)
    {
        rn->txfast = rn->nictxavl;
        rn->txslow = 0;
    }
    else
        rn->nictxavl = rn->txfast;
}
struct fio_context * fio_get_hwl()
{
    if (sysconfig.single_thread == FIO_MODE_BRG_SINGLE)
        return g_contexts;
    return NULL;
}
inline int fio_filter(struct port_range **port_ranges, const char *buf/*, size_t size*/)
{
    if (*port_ranges == NULL)
        return 0;

    uint16_t v = ntohs(*(uint16_t*)(buf+g_recvport_offset));
    struct port_range *ptr;
    for (; (ptr = *port_ranges); port_ranges++)
    {
        if (ptr->pr_st <= v && ptr->pr_ed >= v)
            return 0;
    }

    return -1;
}

inline int fio_tome_byip(struct fio_nic *nic, const char *buf)
{
	uint32_t ndip = ntohl(*(uint32_t*)(buf+g_dip_offset));
	if (fio_map_exist(fio_nic_get_localips(nic), ndip))
	{
		NOD("to me");
		return 1;
	}
	NOD("not me %08X", ndip);
	return 0;
}
 
inline int fio_tome_bymac(struct fio_nic *nic, const char *buf)
{
	struct ether_addr *dmac = (struct ether_addr*)(buf+g_eth_dmac);
    if (sysconfig.prmac_info)
    {
        NOD("tid %d, g_eth_dmac len %d MAC is: %02x:%02x:%02x:%02x:%02x:%02x", 
                NIC_EXTRA_CONTEXT(nic)->me, g_eth_dmac,
                dmac->ether_addr_octet[0], dmac->ether_addr_octet[1], dmac->ether_addr_octet[2],
                dmac->ether_addr_octet[3], dmac->ether_addr_octet[4], dmac->ether_addr_octet[5]);
    }
	//{
	//	char str_x[1500] = {0};   
	//	char str_byte[10];        
	//	int j;  
	//	int rawlen = 12;
	//	for (j = 0; j < rawlen; j++)
	//	{
	//		snprintf(str_byte, 10, "%02X ", (unsigned char)buf[j]);
	//		if (j%6 == 0)
	//		{
	//			strncat(str_x, "\n", 1500);
	//		}
	//		strncat(str_x, str_byte, 1500);
	//	}
	//	D("tid %d, %s", NIC_EXTRA_CONTEXT(nic)->me, str_x);

	//	if (!memcmp(dmac, &nic->if_mac, ETH_ALEN) && 
	//			!memcmp(buf+g_eth_dmac, buf+g_eth_dmac+ETH_ALEN, ETH_ALEN))
	//	{
	//		assert(0);
	//	}
	//}
	if (!memcmp(dmac, &nic->if_mac, ETH_ALEN))
	{
		NOD("tid %d, to me", NIC_EXTRA_CONTEXT(nic)->me);
		return 1;
	}
	NOD("tid %d, not me", NIC_EXTRA_CONTEXT(nic)->me);

	return 0;
}

inline int fio_assort(struct fio_nic *nic, char *pkt, uint16_t *eh_type, uint8_t *ip_type)
{
	NOD("eth_type 0x%x, arp is 0x%x", *(uint16_t*)(pkt+g_eth_type_offset), g_arp_type);
    if (nm_unlikely(fio_map_exist(g_eth_handles, (*eh_type)=*(uint16_t*)(pkt+g_eth_type_offset))))
        return T_FIO_PKT_ETH;

#if 0
    if (!fio_tome_byip(nic, pkt))
#else
    if (!fio_tome_bymac(nic, pkt))
#endif
        return T_FIO_PKT_DISCARD;

    unsigned long pass;
    if (nm_unlikely(fio_map_find_cpy(g_def_handles, (*ip_type=*(uint8_t*)(pkt+g_prototype_offset)), &pass)) &&
            nm_unlikely(pass))
        return T_FIO_PKT_DEF;

    if (nm_likely(fio_map_find_cpy(g_interested_tbl, *ip_type, &pass)) &&
            nm_likely(pass) &&
            nm_likely(!fio_filter(sysconfig.interested_ports_range, pkt)))
        return T_FIO_PKT_INTD;

    return T_FIO_PKT_MAX;
} 

int fio_context_recv_only(struct fio_nic *from, struct fio_nic *to, struct fio_asted_rxs *bufs, uint32_t limit, int accord)
{
    struct netmap_if *nifp = from->nifp;
    struct netmap_ring *rxring;
    int m, recv = 0;

    if ( limit > from->rxavl)
        limit = from->rxavl;

    while (limit > 0)
    {
        rxring = NETMAP_RXRING(nifp, from->cur_rx_head);
        if (rxring->avail == 0)
            from->cur_rx_head = ((from->cur_rx_head+1 == from->qrxlast) ?
                    from->qrxfirst : from->cur_rx_head+1);
        else
        {
            m = fio_recv_pkts(rxring, from, bufs, limit);
            limit -= m;
            recv += m;
        }
    }
    if (recv > 0)
    {
        from->rxavl -= recv;
        from->rxcount += recv;
    }
    return recv;
}

int fio_context_recv(struct fio_nic *from, struct fio_nic *to, struct fio_asted_rxs *bufs, uint32_t limit, int accord)
{
    if ( limit > from->rxavl)
        limit = from->rxavl;
    if (accord == FIO_LIMIT_TO)
    {
        if (limit > to->nictxavl - fio_rbf_occupy(&to->bg_buf))
            limit = to->nictxavl - fio_rbf_occupy(&to->bg_buf);
    }
    else
    {
        if (limit > from->nictxavl - fio_rbf_occupy(&from->bg_buf))
            limit = from->nictxavl - fio_rbf_occupy(&from->bg_buf);
    }

    return from->vtbl.recv(from, bufs, limit);
}

int fio_context_polls(struct fio_poll_data *pd, int sec)
{
    int num_ready = 0, i = 0, j = 0;

#if 0
    for (i = 0; i < pd->num_nic; i++)
    {
        if ( 0 == (pd->nics[i]->dir & (pd->dirs[2*i])) || 
                0 == (pd->nics[i]->dir & (pd->dirs[2*i+1])) )
        {
            D("error direction");
            return -1;
        }
    }

    memset(pd->fds, 0, 2*pd->num_nic*sizeof(struct pollfd));

    for (i = 0; i < pd->num_nic; i++)
    {
        pd->fds[2*i].fd = pd->nics[i]->fd;
        if ( (pd->nics[i]->dir&FIO_DIR_RX) && (pd->dirs[2*i]&FIO_DIR_RX) )
            pd->fds[2*i].events |= POLLIN;
        pd->fds[2*i+1].fd = pd->nics[i]->fd;
        if ( (pd->nics[i]->dir&FIO_DIR_TX) && (pd->dirs[2*i+1]&FIO_DIR_TX) )
            pd->fds[2*i+1].events |= POLLOUT;
    }

    if ( (num_ready=poll(pd->fds, 2*pd->num_nic, sec*1000)) > 0) 
    {
        for (i = 0; i < pd->num_nic; i++)
        {
            if (pd->fds[2*i].revents & POLLIN)
                fio_figure_rxlimit(pd->nics[i]);
            if (pd->fds[2*i+1].revents & POLLOUT)
                fio_figure_txlimit(pd->nics[i]);
        }
    }
#else
    for (i = 0; i < pd->num_nic; i++)
    {
        if ( 0 == (pd->nics[i]->dir & pd->dirs[i]) )
        {
            OD( "error direction");
            return -1;
        }
    }

    for (i = 0; i < pd->num_nic; i++)
    {
        pd->fds[i].events = 0;
        pd->fds[i].fd = pd->nics[i]->fd;
        if ( (pd->nics[i]->dir&FIO_DIR_RX) && (pd->dirs[i]&FIO_DIR_RX) )
            pd->fds[i].events |= POLLIN;
        if ( (pd->nics[i]->dir&FIO_DIR_TX) && (pd->dirs[i]&FIO_DIR_TX) )
            pd->fds[i].events |= POLLOUT;
#ifdef use_old_drive
        else if (pd->nics[i]->spec_tasks & FIO_SPECTSK_POLLOUT)
        {
            pd->fds[i].events |= POLLOUT;
            pd->nics[i]->spec_tasks &= ~FIO_SPECTSK_POLLOUT;
        }
#endif
    }

    for (j = 0; j < pd->num_notify; i++, j++)
    {
        pd->fds[i].events = 0;
        pd->fds[i].fd = pd->notifyfds[j];
        pd->fds[i].events |= POLLIN;
    }

    if ( (num_ready=poll(pd->fds, pd->num_nic+pd->num_notify, sec/**100*/)) > 0) 
    {
        int figure_limit = 0;
        for (i = 0; i < pd->num_notify; i++)
        {
            if (pd->fds[pd->num_nic+i].revents & POLLIN)
            {
                figure_limit = 1;
                NOD("tid %d figure_limit = %d", NIC_EXTRA_CONTEXT(pd->nics[0])->me, figure_limit);
                break;
            }
        }

        for (i = 0; i < pd->num_nic; i++)
        {
            if (figure_limit || (pd->fds[i].revents & POLLIN))
            {
                if (pd->nics[i]->dir & FIO_DIR_RXOPTIMIZED)
                {
#ifdef use_old_drive
                    fio_nic_rxlimit(pd->nics[i]);
                    fio_nic_txlimit(pd->nics[i]);
#else
                    for (j = 0; j < pd->num_nic; j++) 
                    {
                        fio_nic_rxlimit(pd->nics[j]);
                        fio_nic_txlimit(pd->nics[j]);
                    }
                    break;
#endif
                }
                else
                {
                    fio_nic_rxlimit(pd->nics[i]);
                    fio_nic_txlimit(pd->nics[i]);
                }
            }

            if (pd->fds[i].revents & POLLOUT)
            {
                fio_nic_txlimit(pd->nics[i]);
            }
        }
    }
#endif
    return num_ready;
}

static inline void fio_do_spectsk(struct fio_nic *cur_nic, struct fio_context *context)
{
    if ( cur_nic->spec_tasks & FIO_SPECTSK_SNDMAC )
    {
        if (sysconfig.prsys_stack)
            OD( "tid %d nic %s update mac", context->me, cur_nic->alise); 
        if (sysconfig.prmac_info)
            sysconfig.maclog.vtbl.print(&sysconfig.maclog, "tid %d nic %s update mac\n", 
                    context->me, cur_nic->alise);
        fio_nic_snd_alldomain_mac(cur_nic);
        cur_nic->spec_tasks &= ~FIO_SPECTSK_SNDMAC;
    }
    if ( cur_nic->spec_tasks & FIO_SPECTSK_UPDATEMP)
    {
        assert(cur_nic == context->nics);
        if (sysconfig.prsys_stack)
            OD( "tid %d nic %s update mp", context->me, cur_nic->alise); 
        if (sysconfig.prmac_info)
            sysconfig.maclog.vtbl.print(&sysconfig.maclog, "tid %d nic %s update mp\n", 
                    context->me, cur_nic->alise);
        int i;
        for (i = 0; i < context->num_nic; i++)
        {
            //{
            //	time_t t;  

            //	t = time(NULL);  
            //	fprintf(stderr, "tid %d nic %d The number of seconds since January 1, 1970 is %ld\n",
            //			context->me, i, t);  
            //}
            fio_mp_update_nic(&context->nics[i].mp_ipinfo, context->nics[i].buddy_macs);
        }
        fio_route_context_update(&context->route_context, context->nics, &context->defroute_context);
        cur_nic->spec_tasks &= ~FIO_SPECTSK_UPDATEMP;
    }
    if ( cur_nic->spec_tasks & FIO_SPECTSK_CLEARMAC )
    {
        if (sysconfig.prsys_stack)
            OD( "tid %d nic %s clear buddy mac", context->me, cur_nic->alise); 
        if (sysconfig.prmac_info)
            sysconfig.maclog.vtbl.print(&sysconfig.maclog, "tid %d nic %s clear buddy mac\n", 
                    context->me, cur_nic->alise);
        struct fio_notify_msg msg;
        msg.msg_id = FIO_MSG_CLEAR_MACPOOL;
        msg.nic_id = cur_nic->type_id;
        msg.abstract = 0UL;
        fio_stack_notify_systhread(cur_nic, &msg);
        cur_nic->spec_tasks &= ~FIO_SPECTSK_CLEARMAC;
    }
}

static inline void fio_do_copytsk(struct fio_nic *cur_nic, struct fio_context *context, int enablesys)
{
    fio_mac_check_cache(cur_nic);
    fio_do_spectsk(cur_nic, context);
    fio_nic_send_arpbuf(cur_nic);
    fio_nic_send_txbuf(cur_nic);
    if (enablesys)
        fio_nic_send_sys_rxbuf(cur_nic);
    fio_nic_send_upsndbuf(cur_nic);
}

#ifdef use_old_drive
//#pragma  message("oooooold driver")
#define TOUCH_POLLOUT(nics, num) for (i = 0; i < num; i++) \
				if (nics[i]->nictxavl < 1) \
			nics[i]->spec_tasks |= FIO_SPECTSK_POLLOUT; 
#endif

static void * fio_bridge(void *t)
{
    struct fio_context *context = (struct fio_context *)t;
    uint32_t needcp_rx;
    struct fio_nic *nics[2];
    struct pollfd fds[4+1];
    uint32_t dirs[4];
    struct fio_asted_rxs rxbufs[T_FIO_PKT_DISCARD];
    int limit = MAX_PKT_COPY_ONCE;
    int m, num_ready, i, num_nic = context->num_nic, enable_sysstack = sysconfig.enable_sysstack;
    int dirm[][2] = { {0, 1}, {1, 0}};
    struct fio_nic *rn, *tn, *cur_nic;
    struct fio_poll_data pd;
    struct fio_notify_msg msg;

    memset(rxbufs, 0, T_FIO_PKT_DISCARD*sizeof(struct fio_asted_rxs));
    for (i = T_FIO_PKT_INTD; i < T_FIO_PKT_DISCARD; i++)
        rxbufs[i].cb_type = i;

    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = (FIO_DIR_RX/*|FIO_DIR_TX*/);
    }

    if (num_nic == 1)
    {
        dirm[0][0] = 0;
        dirm[0][1] = 0;
        nics[1] = nics[0];
    }

    needcp_rx = (nics[0]->spec_tasks&FIO_SPECTSK_COPYRX);

    NOD("tid %d Wait %d secs for phy reset", context->me, 5);
    sleep(5);
    OD( "tid %d Ready... num_nic %d", context->me, num_nic);
    gettimeofday(&context->tic, NULL);

    NOD("my rxbufs addr %p", rxbufs);

    pd.nics = nics;
    pd.fds = fds;
    pd.dirs = dirs;
    pd.num_nic = num_nic;
    pd.notifyfds = context->notifyfds;
    pd.num_notify = 0;
    if (enable_sysstack)
        pd.num_notify = 1;

    while (sysconfig.working) 
    {
#ifdef use_old_drive
	    TOUCH_POLLOUT(nics, num_nic)
#endif

        if ( (num_ready=context->vtbl.polls(&pd, 500)) < 0) 
        {
            gettimeofday(&context->toc, NULL);
            context->toc.tv_sec -= 1; /* Subtract timeout time. */
                OD( "<error> poll error!!!!!");
            continue;
        }

        if (num_ready == 0)
        {
            for (i = 0; i < num_nic; i++)
            {
                cur_nic = nics[i];
                if (fio_rbf_occupy(&cur_nic->tx_buf) ||
                        fio_rbf_occupy(&cur_nic->arp_buf) ||
                        fio_rbf_occupy(&cur_nic->upsnd_buf) )
                {
                    cur_nic->nifp->ni_flag |= NIF_RXNOSLEEP;
                    NOD("tid %d set NIF_RXNOSLEEP", context->me);
                }
                if (fio_nic_txlimit(cur_nic))
                    fio_do_copytsk(cur_nic, context, enable_sysstack);
            }
            continue;
        }

        m = 0; 
        for (i = 0; i < num_nic; i++)
        {
            cur_nic = nics[i];
            fio_do_copytsk(cur_nic, context, enable_sysstack);
        }

        for (i = 0; i < num_nic; i++)
        {
            rn = nics[dirm[i][0]];
            tn = nics[dirm[i][1]];
            if (num_nic == 1)
            {
                //tn->nictxavl = tn->txfast;
                fio_refresh_nictxlimit(tn);
            }

            if (rn->rxavl > 0 && tn->nictxavl - fio_rbf_occupy(&tn->bg_buf) > 0)
            {
                NOD("tid %d %s, rx %u, tx %u, rnbgoc %d, tnbgoc %d", context->me, rn->alise, 
                        rn->rxavl, tn->nictxavl, fio_rbf_occupy(&rn->bg_buf), fio_rbf_occupy(&tn->bg_buf));
                m = context->vtbl.recv(rn, tn, rxbufs, limit, FIO_LIMIT_TO);
                if (m > 0)
                {
                    process_data(rn, tn, nics, num_nic, rxbufs, TXDATA_TYPE_REALTIME, needcp_rx);
                    //cansleep = 0;
                }
            }
            if (num_nic == 1)
            {
                tn->nictxavl += tn->txslow;
            }
            fio_nic_send_bgbuf(tn);
            fio_nic_send_arpbuf(tn);
            fio_nic_send_txbuf(tn);
            if (num_nic > 1)
            {
                fio_nic_send_bgbuf(rn);
                fio_nic_send_arpbuf(rn);
                fio_nic_send_txbuf(rn);
            }
            if (fio_rbf_occupy(&tn->tx_buf) ||
                    fio_rbf_occupy(&tn->arp_buf) ||
                    fio_rbf_occupy(&tn->upsnd_buf) )
                tn->nifp->ni_flag |= NIF_RXNOSLEEP;
        }

        for (i = 0; i < pd.num_notify; i++)
        {
            if (fds[num_nic+i].revents & POLLIN)
            {
                if (read(context->notifyfds[i], &msg, sizeof(msg)) < 0) 
            {
                    OD( "tid %d error %d reading sockpair", context->me, errno);
            }
                else
                {
                    fio_handle_ctmsg(context, &msg);
                }
            }
        }
    }

    context->used = 0;
    OD( "I'll shutdown");

    return (NULL);
}

static void * fio_bridge_in_(void *t)
{
    struct fio_context *context = (struct fio_context *)t;
    uint32_t needcp_rx;
    struct fio_nic *nics[2];
    struct pollfd fds[4+1];
    uint32_t dirs[4];
    struct fio_asted_rxs rxbufs[T_FIO_PKT_DISCARD];
    int limit = MAX_PKT_COPY_ONCE/*, usefast = 0*/;
    int m, num_ready, i, num_nic = context->num_nic;
    const int fast_time = 50, enable_sysstack = sysconfig.enable_sysstack;
    //const int slow_time = 2000;
    int dirm[][2] = { {0, 1}, {1, 0}};
    struct fio_nic *rn, *tn, *cur_nic;
    struct fio_poll_data pd;
    struct fio_notify_msg msg;

    memset(rxbufs, 0, T_FIO_PKT_DISCARD*sizeof(struct fio_asted_rxs));
    for (i = T_FIO_PKT_INTD; i < T_FIO_PKT_DISCARD; i++)
        rxbufs[i].cb_type = i;
    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = (FIO_DIR_RX);
    }

    if (num_nic == 1)
    {
        dirm[0][0] = 0;
        dirm[0][1] = 0;
        nics[1] = nics[0];
    }

    needcp_rx = (nics[0]->spec_tasks&FIO_SPECTSK_COPYRX);

    NOD("tid %d Wait %d secs for phy reset", context->me, 5);
    sleep(5);
    OD( "tid %d Ready... num_nic %d", context->me, num_nic);
    gettimeofday(&context->tic, NULL);

    NOD("my rxbufs addr %p", rxbufs);

    pd.nics = nics;
    pd.fds = fds;
    pd.dirs = dirs;
    pd.num_nic = num_nic;
    pd.notifyfds = context->notifyfds;
    pd.num_notify = 0;
    if (enable_sysstack)
        pd.num_notify = 1;
    if (sysconfig.enable_slog)
    {
    for (i = 0; i < num_nic; i++)
    {
        //fio_slog_snd(nics[i], "start", strlen("start"));
        fio_slog_snd_format(nics[i], "[%s] start", nics[i]->alise);
        }
    }

    while (sysconfig.working) 
    {
#ifdef use_old_drive
        TOUCH_POLLOUT(nics, num_nic)
#endif
        //if (usefast)
        num_ready = context->vtbl.polls(&pd, fast_time);
        //else
        //num_ready = context->vtbl.polls(nics, fds, dirs, num_nic, slow_time);
        //usefast = 0;

        if ( num_ready < 0) 
        {
            gettimeofday(&context->toc, NULL);
            context->toc.tv_sec -= 1; /* Subtract timeout time. */
            OD( "<error> poll error!!!!!");
            continue;
        }

        if (num_ready == 0)
        {
            for (i = 0; i < num_nic; i++)
            {
                cur_nic = nics[i];
                if (fio_rbf_occupy(&cur_nic->tx_buf) ||
                        fio_rbf_occupy(&cur_nic->arp_buf) ||
                        fio_rbf_occupy(&cur_nic->upsnd_buf) )
                {
                    cur_nic->nifp->ni_flag |= NIF_RXNOSLEEP;
                    //usefast = 1;
                    NOD("tid %d set NIF_RXNOSLEEP", context->me);
                }
                if (fio_nic_txlimit(cur_nic))
                    fio_do_copytsk(cur_nic, context, enable_sysstack);
            }
            continue;
        }

        m = 0; 
        for (i = 0; i < num_nic; i++)
        {
            cur_nic = nics[i];
            fio_do_copytsk(cur_nic, context, enable_sysstack);
        }

        for (i = 0; i < num_nic; i++)
        {
            rn = nics[dirm[i][0]];
            tn = nics[dirm[i][1]];

            if (num_nic == 1)
            {
                //rn->nictxavl = rn->txfast;
                fio_refresh_nictxlimit(rn);
            }

            if (rn->rxavl > 0 && rn->nictxavl - fio_rbf_occupy(&rn->bg_buf) > 0)
            {
                NOD("tid %d %s, rx %u, tx %u, rnbgoc %d, tnbgoc %d", context->me, rn->alise, 
                        rn->rxavl, tn->nictxavl, fio_rbf_occupy(&rn->bg_buf), fio_rbf_occupy(&tn->bg_buf));
                m = context->vtbl.recv(rn, tn, rxbufs, limit, FIO_LIMIT_FROM);
                if (m > 0)
                    process_data(rn, tn, nics, num_nic, rxbufs, TXDATA_TYPE_REALTIME, needcp_rx);
            }

            if (num_nic == 1)
                rn->nictxavl += rn->txslow;

            fio_nic_send_bgbuf(rn);
            fio_nic_send_arpbuf(rn);
            fio_nic_send_txbuf(rn);
            if (num_nic > 1)
            {
                fio_nic_send_bgbuf(tn);
                fio_nic_send_arpbuf(tn);
                fio_nic_send_txbuf(tn);
            }
            if (fio_rbf_occupy(&rn->tx_buf) || 
                    fio_rbf_occupy(&rn->arp_buf) ||
                    fio_rbf_occupy(&rn->upsnd_buf) )
            {
                rn->nifp->ni_flag |= NIF_RXNOSLEEP;
                //usefast = 1;
            }
        }

        for (i = 0; i < pd.num_notify; i++)
        {
            if (fds[num_nic+i].revents & POLLIN)
            {
                if (read(context->notifyfds[i], &msg, sizeof(msg)) < 0) 
            {
                    OD( "tid %d error %d reading sockpair", context->me, errno);
            }
                else
                {
                    fio_handle_ctmsg(context, &msg);
                }
            }
        }
    }

    context->used = 0;
    OD( "I'll shutdown");
    return (NULL);
}
static void * fio_brg_single(void *t)
{
    struct fio_context *context = (struct fio_context *)t;
    uint32_t needcp_rx;
    struct fio_nic *nics[2];
    struct pollfd fds[4+2];
    uint32_t dirs[4];
    struct fio_asted_rxs rxbufs[T_FIO_PKT_DISCARD];
    int limit = MAX_PKT_COPY_ONCE;
    int m, num_ready, i, num_nic = context->num_nic;
    const int fast_time = 50, enable_sysstack = sysconfig.enable_sysstack;
    int dirm[][2] = { {0, 1}, {1, 0}};
    struct fio_nic *rn, *tn, *cur_nic;
    struct fio_poll_data pd;
    struct fio_notify_msg msg;
    memset(rxbufs, 0, T_FIO_PKT_DISCARD*sizeof(struct fio_asted_rxs));
    for (i = T_FIO_PKT_INTD; i < T_FIO_PKT_DISCARD; i++)
        rxbufs[i].cb_type = i;
    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = (FIO_DIR_RX);
    }
    if (num_nic == 1)
    {
        dirm[0][0] = 0;
        dirm[0][1] = 0;
        nics[1] = nics[0];
    }
    needcp_rx = (nics[0]->spec_tasks&FIO_SPECTSK_COPYRX);
    NOD("tid %d Wait %d secs for phy reset", context->me, 5);
    sleep(5);
    OD( "tid %d Ready... num_nic %d", context->me, num_nic);
    gettimeofday(&context->tic, NULL);
    NOD("my rxbufs addr %p", rxbufs);
    pd.nics = nics;
    pd.fds = fds;
    pd.dirs = dirs;
    pd.num_nic = num_nic;
    pd.notifyfds = context->notifyfds;
    pd.num_notify = 0;
    if (enable_sysstack)
        pd.num_notify = 1;
    pd.num_notify++;
    if (sysconfig.enable_slog)
        for (i = 0; i < num_nic; i++)
            fio_slog_snd_format(nics[i], "[%s] start", nics[i]->alise);

    while (sysconfig.working) 
    {
#ifdef use_old_drive
        TOUCH_POLLOUT(nics, num_nic)
#endif
        num_ready = context->vtbl.polls(&pd, fast_time);
        if ( num_ready < 0) 
        {
            gettimeofday(&context->toc, NULL);
            context->toc.tv_sec -= 1; /* Subtract timeout time. */
            OD( "<error> poll error!!!!!");
            continue;
        }
        if (num_ready == 0)
        {
            for (i = 0; i < num_nic; i++)
            {
                cur_nic = nics[i];
                if (fio_rbf_occupy(&cur_nic->tx_buf) ||
                        fio_rbf_occupy(&cur_nic->arp_buf) ||
                        fio_rbf_occupy(&cur_nic->upsnd_buf) )
                {
                    cur_nic->nifp->ni_flag |= NIF_RXNOSLEEP;
                    NOD("tid %d set NIF_RXNOSLEEP", context->me);
                }
                if (fio_nic_txlimit(cur_nic))
                    fio_do_copytsk(cur_nic, context, enable_sysstack);
            }
            continue;
        }
        m = 0; 
        for (i = 0; i < num_nic; i++)
        {
            cur_nic = nics[i];
            fio_do_copytsk(cur_nic, context, enable_sysstack);
        }
        for (i = 0; i < num_nic; i++)
        {
            rn = nics[dirm[i][0]];
            tn = nics[dirm[i][1]];
            if (num_nic == 1)
            {
                //rn->nictxavl = rn->txfast;
                fio_refresh_nictxlimit(rn);
            }
            if (rn->rxavl > 0 && rn->nictxavl - fio_rbf_occupy(&rn->bg_buf) > 0)
            {
                m = context->vtbl.recv(rn, tn, rxbufs, limit, FIO_LIMIT_FROM);
                if (m > 0)
                    process_data(rn, tn, nics, num_nic, rxbufs, TXDATA_TYPE_REALTIME, needcp_rx);
            }
            if (num_nic == 1)
                rn->nictxavl += rn->txslow;
            fio_nic_send_bgbuf(rn);
            fio_nic_send_arpbuf(rn);
            fio_nic_send_txbuf(rn);
            if (num_nic > 1)
            {
                fio_nic_send_bgbuf(tn);
                fio_nic_send_arpbuf(tn);
                fio_nic_send_txbuf(tn);
            }
            if (fio_rbf_occupy(&rn->tx_buf) ||
                    fio_rbf_occupy(&rn->arp_buf) ||
                    fio_rbf_occupy(&rn->upsnd_buf) )
                rn->nifp->ni_flag |= NIF_RXNOSLEEP;
        }
        for (i = 0; i < pd.num_notify; i++)
        {
            if (fds[num_nic+i].revents & POLLIN)
            {
                if (read(context->notifyfds[i], &msg, sizeof(msg)) < 0) 
                {
                    OD( "tid %d error %d reading sockpair", context->me, errno);
                }
                else
                {
                    //if (i == 1)
                    //    FLOG2_TRACE(g_log2, "tid %d reading %d sockpair tesssssssssst.....", context->me, i);
                    fio_handle_ctmsg(context, &msg);
                }
            }
        }
    }
    context->used = 0;
    OD( "I'll shutdown");

    return (NULL);
}

//双网卡
static void * fio_2cards(void *t)
{
    struct fio_context *context = (struct fio_context *)t;
    uint32_t needcp_rx;
    struct fio_nic *nics[2];
    struct pollfd fds[4+1];
    uint32_t dirs[4];
    struct fio_asted_rxs rxbufs[T_FIO_PKT_DISCARD];
    int limit = MAX_PKT_COPY_ONCE;
    int m, num_ready, i, num_nic = context->num_nic, enable_sysstack = sysconfig.enable_sysstack;
    int dirm[][2] = { {0, 0}, {1, 1}};
    struct fio_nic *rn, *tn, *cur_nic;
    struct fio_poll_data pd;
    struct fio_notify_msg msg;

    memset(rxbufs, 0, T_FIO_PKT_DISCARD*sizeof(struct fio_asted_rxs));
    for (i = T_FIO_PKT_INTD; i < T_FIO_PKT_DISCARD; i++)
        rxbufs[i].cb_type = i;

    for (i = 0; i < num_nic; i++)
    {
        nics[i] = &context->nics[i];
        dirs[i] = (FIO_DIR_RX/*|FIO_DIR_TX*/);
        nics[1] = nics[0];
    }

    needcp_rx = (nics[0]->spec_tasks&FIO_SPECTSK_COPYRX);

    NOD("tid %d Wait %d secs for phy reset", context->me, 5);
    sleep(5);
    OD( "tid %d Ready... num_nic %d", context->me, num_nic);
    gettimeofday(&context->tic, NULL);

    NOD("my rxbufs addr %p", rxbufs);

    pd.nics = nics;
    pd.fds = fds;
    pd.dirs = dirs;
    pd.num_nic = num_nic;
    pd.notifyfds = context->notifyfds;
    pd.num_notify = 0;
    if (enable_sysstack)
        pd.num_notify = 1;

    while (sysconfig.working) 
    {
        if ( (num_ready=context->vtbl.polls(&pd, 500)) < 0) 
        {
            gettimeofday(&context->toc, NULL);
            context->toc.tv_sec -= 1; /* Subtract timeout time. */
            OD( "<error> poll error!!!!!");
            continue;
        }

        if (num_ready == 0)
        {
            for (i = 0; i < num_nic; i++)
            {
                cur_nic = nics[i];
                if (fio_rbf_occupy(&cur_nic->tx_buf) ||
                        fio_rbf_occupy(&cur_nic->arp_buf) ||
                        fio_rbf_occupy(&cur_nic->upsnd_buf) )
                {
                    cur_nic->nifp->ni_flag |= NIF_RXNOSLEEP;
                    NOD("tid %d set NIF_RXNOSLEEP", context->me);
                }
                if (fio_nic_txlimit(cur_nic))
                    fio_do_copytsk(cur_nic, context, enable_sysstack);
            }
            continue;
        }

        m = 0; 

        for (i = 0; i < num_nic; i++)
        {
            cur_nic = nics[i];
            fio_do_copytsk(cur_nic, context, enable_sysstack);
        }

        for (i = 0; i < num_nic; i++)
        {
            rn = nics[dirm[i][0]];
            tn = nics[dirm[i][1]];
            if (rn->rxavl > 0 && tn->nictxavl - fio_rbf_occupy(&tn->bg_buf) > 0)
            {
                m = context->vtbl.recv(rn, tn, rxbufs, limit, FIO_LIMIT_TO);
                NOD("m = %d, my rxbufs1 %p", m, rxbufs);
                if (m > 0)
                {
                    NOD("my rxbufs2 = %p", rxbufs);
                    process_data(rn, tn, nics, num_nic, rxbufs, TXDATA_TYPE_REALTIME, needcp_rx);
                    NOD("my rxbufs3 = %p", rxbufs);
                }
            }
            fio_nic_send_bgbuf(tn);
            fio_nic_send_arpbuf(tn);
            fio_nic_send_txbuf(tn);
            if (fio_rbf_occupy(&tn->tx_buf) ||
                    fio_rbf_occupy(&tn->arp_buf) ||
                    fio_rbf_occupy(&tn->upsnd_buf) )
                tn->nifp->ni_flag |= NIF_RXNOSLEEP;
        }

        for (i = 0; i < pd.num_notify; i++)
        {
            if (fds[num_nic+i].revents & POLLIN)
            {
                if (read(context->notifyfds[i], &msg, sizeof(msg)) < 0) 
            {
                    OD( "tid %d error %d reading sockpair", context->me, errno);
            }
                else
                {
                    fio_handle_ctmsg(context, &msg);
                }
            }
        }
    }

    context->used = 0;
    OD( "I'll shutdown");

    return (NULL);
}
static int fio_standby_upsnd()
{
    if (FIO_MODE_BRG_SINGLE != sysconfig.single_thread)
        return 0;
    int j;
    for (j = 0; j < sysconfig.nthreads; j++) 
    {
        int sockets[2];
        if (fio_usocketpair( &sysconfig.if_ips[0], 60001, sockets) < 0)
            OD( "tid %d error %d on fio_socketpair mode %d\n", j, errno, FIO_MODE_BRG_SINGLE);
        if (sysconfig.enable_sysstack)
        {
            g_contexts[j].notifyfds[1] = sockets[0];
            g_contexts[j].notifyfds[2] = sockets[1];
        }
        else
        {
            g_contexts[j].notifyfds[0] = sockets[0];
            g_contexts[j].notifyfds[1] = sockets[1];
        }
    }
    return 0;
}
static int fio_standby_ss(int me, char **alises, void *mmap_addr)
{
    if (!sysconfig.enable_sysstack)
        return 0;
    int j;
    g_syscontext = calloc(1, sizeof(*g_syscontext));
    g_syscontext->me = me;
    g_syscontext->vtbl.polls = fio_context_polls;
    g_syscontext->vtbl.recv = fio_context_recv;
    g_syscontext->num_nic = sysconfig.num_nic;
    for (j = 0; j < FIO_MAX_NICS; j++) 
        g_syscontext->nics[j].type_id = j;
    if (mmap_addr)
        g_syscontext->nics[0].mmap_addr = mmap_addr;
    if (fio_nic_open(&g_syscontext->nics[0], alises[0], sysconfig.nic_names[0],
                FIO_SYS_RING, (FIO_DIR_RXOPTIMIZED|FIO_DIR_TX), g_syscontext))
    {
        OD( "tid %d open %s %s error!!", g_syscontext->me, alises[0], sysconfig.nic_names[0]);
        goto err;
    }
    g_syscontext->nics[0].vtbl.send_sys = fio_stack_send_nic;
    if (g_syscontext->num_nic > 1)
    {
        g_syscontext->nics[1].mmap_addr = g_syscontext->nics[0].mmap_addr;
        if ( fio_nic_open(&g_syscontext->nics[1], alises[1], sysconfig.nic_names[1], 
                    FIO_SYS_RING, (FIO_DIR_RXOPTIMIZED|FIO_DIR_TX), g_syscontext))
        {
            OD( "tid %d open %s %s error!!", g_syscontext->me, alises[1], sysconfig.nic_names[1]);
            goto err;
        }
        g_syscontext->nics[1].vtbl.send_sys = fio_stack_send_nic;
    }
    g_syscontext->notifyfds = calloc(sysconfig.nthreads, sizeof(int));
    for (j = 0; j < sysconfig.nthreads; j++) 
    {
        int sockets[2];
        if (fio_usocketpair( &sysconfig.if_ips[0], 60000, sockets) < 0)
            OD( "tid %d error %d on fio_socketpair\n", j, errno);
        g_contexts[j].notifyfds = malloc(3*sizeof(int)); //FIO_MODE_BRG_SINGLE的情况要两个
        g_contexts[j].notifyfds[0] = sockets[0];
        g_syscontext->notifyfds[j] = sockets[1];
    }
    return 0;
err:
    return -1;
}
int fio_standby_rr(struct fio_context *ct)
{
    int j;
    ct->buddy_macs = fio_map_create(sizeof(struct fio_mac_pkt_cache));
    fio_map_init(ct->buddy_macs, FIO_MAX_MAC_NIC, FIO_MAX_MAC_NIC, "context_othermacs"); 
    ct->pktcache_list_nic = calloc(ct->num_nic, sizeof(struct fio_list_head));
    {
        for (j = 0; j < ct->num_nic; j++) 
        {
            FIO_INIT_LIST_HEAD(ct->pktcache_list_nic+j);
            strncpy(ct->nics[j].mp_ipinfo.nic_name, sysconfig.nic_names[j], FIO_MAX_NAME_LEN);
            FIO_INIT_LIST_HEAD(&ct->nics[j].mp_ipinfo.mask_sort);
            ct->nics[j].mp_ipinfo.mask_cnt = 0;
            ct->nics[j].mp_ipinfo.if_localaddrs = fio_map_create(sizeof(char));
            fio_map_init(ct->nics[j].mp_ipinfo.if_localaddrs, FIO_MAX_IP_NIC, FIO_MAX_IP_NIC, "nic_loaclips"); 
            ct->nics[j].mp_ipinfo.map_arpa = fio_mapstr_create(sizeof(char));
            fio_mapstr_init(ct->nics[j].mp_ipinfo.map_arpa, FIO_MAX_IP_NIC, FIO_MAX_IP_NIC, "nic_maparpa"); 
            ct->nics[j].buddy_macs = ct->buddy_macs;
            ct->nics[j].route_context = &ct->route_context;
            ct->nics[j].defroute_context = &ct->defroute_context;
            ct->nics[j].buddy_mac_pkt_factory= &ct->buddy_mac_pkt_factory;
            ct->nics[j].pktcache_list_nic = ct->pktcache_list_nic;
        }
        fio_mp_init(sysconfig.nic_names, ct->num_nic);
        fio_route_context_init(&ct->route_context);
        fio_route_defroute_init(&ct->defroute_context);
        fio_mac_pkt_factory_init(&ct->buddy_mac_pkt_factory, ct->num_nic);
        for (j = 0; j < ct->num_nic; j++) 
        {
            fio_mp_update_nic(&ct->nics[j].mp_ipinfo, ct->nics[j].buddy_macs);
        }
        fio_route_context_update(&ct->route_context, ct->nics, &ct->defroute_context);
    }
    return 0;
}

int fio_enter_mode3(int *txqueues)
{
    int i, jj;
    void *mmap_addr = NULL;
    struct fio_context *_ct = NULL;
    struct pktudp *pkts[] = {&g_pkt4test, &g_pkt4test2};
    g_contexts = calloc(sysconfig.num_nic, sizeof(*g_contexts));
    __initialize_dns_packet("12345678901234567890", 20);
    sysconfig.nthreads = sysconfig.num_nic;
    for (i = 0; i < sysconfig.num_nic; i++) 
    {
        _ct = g_contexts+i;
        _ct->me = i;
        _ct->vtbl.polls = fio_context_polls;
        _ct->vtbl.recv = NULL;
        _ct->num_nic = 1;
        for (jj = 0; jj < FIO_MAX_NICS; jj++) 
            _ct->nics[jj].type_id = jj;
        if (mmap_addr)
            _ct->nics[0].mmap_addr = mmap_addr;
        if (fio_nic_open(&_ct->nics[0], sysconfig.nic_names[i], sysconfig.nic_names[i],
                    FIO_LSTN_ALL_RING, FIO_DIR_TX, _ct) )
        {
            OD( "tid %d open %s %s error!!", i, sysconfig.nic_names[i], sysconfig.nic_names[i]);
            goto err;
        }
        if (!mmap_addr)
            mmap_addr = g_contexts[i].nics[0].mmap_addr;
#ifdef __old_mac_mgr__
        fio_mac_init(&_ct->nics[0].mac_mgr, sysconfig.vlans[i]/*, sysconfig.mac_lifetime*/);
#endif
        fio_nic_init(&_ct->nics[0], *txqueues*TXBUF_SIZE, *txqueues*sysconfig.burst);
        _ct->nics[0].pkt_test = pkts[i];
        _ct->used = 1;
        _ct->n_test = 99999;
    }
    return 0;
err:
    return -1;
}
int fio_enter_mode2(int *txqueues)
{
    int i, j;
    uint32_t dirs[2] = {FIO_DIR_TX, FIO_DIR_RX};
    char *alises[2] = {"send", "recv"};
    void *mmap_addr = NULL;

    g_contexts = calloc(sysconfig.nthreads, sizeof(*g_contexts));
    __initialize_dns_packet("12345678901234567890", 20);
    for (i = 0; i < sysconfig.nthreads; i++) 
    {
        g_contexts[i].me = i;
        g_contexts[i].vtbl.polls = fio_context_polls;
        g_contexts[i].vtbl.recv = fio_context_recv_only;
        g_contexts[i].num_nic = sysconfig.num_nic;

        for (j = 0; j < FIO_MAX_NICS; j++) 
            g_contexts[i].nics[j].type_id = j;

        fio_standby_rr(&g_contexts[i]);
        if (i == 0)
            for (j = 0; j < g_contexts[i].num_nic; j++)
                fio_route_resolve_defgw_mac(&g_contexts[i].nics[j], &sysconfig.if_macs[j], sysconfig.nic_names[j]);
        if (mmap_addr)
            g_contexts[i].nics[0].mmap_addr = mmap_addr;
        if (fio_nic_open(&g_contexts[i].nics[0], alises[i], sysconfig.nic_names[0],
                    FIO_LSTN_ALL_RING, dirs[i], g_contexts+i) )
        {
            OD( "tid %d open %s %s error!!", i, alises[i], sysconfig.nic_names[0]);
            goto err;
        }
        if (!mmap_addr)
            mmap_addr = g_contexts[i].nics[0].mmap_addr;
        
        if (g_contexts[i].num_nic > 1)
        {
            g_contexts[i].nics[1].mmap_addr = g_contexts[i].nics[0].mmap_addr;
            if ( fio_nic_open(&g_contexts[i].nics[1], alises[i], sysconfig.nic_names[1], 
                        FIO_LSTN_ALL_RING, dirs[i], g_contexts+i) )
            {
                OD( "tid %d open %s %s error!!", i, alises[i], sysconfig.nic_names[1]);
                goto err;
            }
        }

        struct pktudp *pkts[] = {&g_pkt4test, &g_pkt4test2};
        for (j = 0; j < g_contexts[i].num_nic; j++) 
        {
#ifdef __old_mac_mgr__
            fio_mac_init(&g_contexts[i].nics[j].mac_mgr, sysconfig.vlans[j]);
#endif
            fio_nic_init(&g_contexts[i].nics[j], *txqueues*TXBUF_SIZE, *txqueues*sysconfig.burst);
            g_contexts[i].nics[j].pkt_test = pkts[j];
        }
        //g_contexts[i].mac_lifetime = sysconfig.mac_lifetime;
        g_contexts[i].used = 1;
        g_contexts[i].n_test = 99999;
    }
    fio_mac_init_time_mgr(&sysconfig);
    if (fio_standby_ss(i, alises, mmap_addr) || fio_standby_upsnd())
        goto err;
    return 0;
err:
    return -1;
}

int fio_enter_mode0(int *txqueues)
{
    int i, j;
    char *alises[2] = {"in", "out"};
    uint32_t dirs[2] = {(FIO_DIR_RXOPTIMIZED|FIO_DIR_TX), (FIO_DIR_RXOPTIMIZED|FIO_DIR_TX)};
    void *mmap_addr = NULL;
    g_contexts = calloc(sysconfig.nthreads, sizeof(*g_contexts));
    __initialize_dns_packet("12345678901234567890", 20);
    for (i = 0; i < sysconfig.nthreads; i++) 
    {

        g_contexts[i].me = i;
        g_contexts[i].vtbl.polls = fio_context_polls;
        g_contexts[i].vtbl.recv = fio_context_recv;
        g_contexts[i].num_nic = sysconfig.num_nic;

        for (j = 0; j < FIO_MAX_NICS; j++) 
            g_contexts[i].nics[j].type_id = j;

        fio_standby_rr(&g_contexts[i]);
        if (i == 0)
            for (j = 0; j < g_contexts[i].num_nic; j++)
                fio_route_resolve_defgw_mac(&g_contexts[i].nics[j], &sysconfig.if_macs[j], sysconfig.nic_names[j]);

        if (mmap_addr)
            g_contexts[i].nics[0].mmap_addr = mmap_addr;
        if (fio_nic_open(&g_contexts[i].nics[0], alises[0], sysconfig.nic_names[0], 
                    (sysconfig.single_thread != FIO_MODE_BRG_SINGLE && sysconfig.single_thread != FIO_MODE_1_SND_AND_RCV) ? i : FIO_LSTN_ALL_RING, 
                    dirs[0], g_contexts+i) )
        {
            OD( "tid %d open %s %s error!!", i, alises[0], sysconfig.nic_names[0]);
            goto err;
        }
        if (!mmap_addr)
            mmap_addr = g_contexts[i].nics[0].mmap_addr;
        if (g_contexts[i].num_nic > 1)

        {
            g_contexts[i].nics[1].mmap_addr = g_contexts[i].nics[0].mmap_addr;
            if ( fio_nic_open(&g_contexts[i].nics[1], alises[1], sysconfig.nic_names[1], 
                        (sysconfig.single_thread != FIO_MODE_BRG_SINGLE && sysconfig.single_thread != FIO_MODE_1_SND_AND_RCV) ? i : FIO_LSTN_ALL_RING, 
                        dirs[1], g_contexts+i) )
            {
                OD( "tid %d open %s %s error!!", i, alises[1], sysconfig.nic_names[1]);
                goto err;
            }
        }

        struct pktudp *pkts[] = {&g_pkt4test, &g_pkt4test};
        if (sysconfig.single_thread == FIO_MODE_2CARDS)
        {
            pkts[0] = &g_pkt4test;
            pkts[1] = &g_pkt4test2;
        }

        if (sysconfig.nthreads == 2)
        {
            for (j = 0; j < g_contexts[i].num_nic; j++) 
            {
#ifdef __old_mac_mgr__
                fio_mac_init(&g_contexts[i].nics[j].mac_mgr, sysconfig.vlans[j]);
#endif
                fio_nic_init(&g_contexts[i].nics[j], txqueues[j]*TXBUF_SIZE, txqueues[j]*sysconfig.burst);
                g_contexts[i].nics[j].pkt_test = pkts[j];
            }
        }
        else if (sysconfig.nthreads > 1)
        {
            for (j = 0; j < g_contexts[i].num_nic; j++) 
            {
#ifdef __old_mac_mgr__
                fio_mac_init(&g_contexts[i].nics[j].mac_mgr, sysconfig.vlans[j]);
#endif
                fio_nic_init(&g_contexts[i].nics[j], TXBUF_SIZE, sysconfig.burst);
                g_contexts[i].nics[j].pkt_test = pkts[j];
            }
        }
        else
        {
            for (j = 0; j < g_contexts[i].num_nic; j++) 
            {
#ifdef __old_mac_mgr__
                fio_mac_init(&g_contexts[i].nics[j].mac_mgr, sysconfig.vlans[j]);
#endif
                fio_nic_init(&g_contexts[i].nics[j], txqueues[j]*TXBUF_SIZE, txqueues[j]*sysconfig.burst);
                g_contexts[i].nics[j].pkt_test = pkts[j];
            }
        }
        //g_contexts[i].mac_lifetime = sysconfig.mac_lifetime;
        g_contexts[i].used = 1;
        g_contexts[i].n_test = 99999;


        NOD("pkt[42] = %d", *(int*)&g_contexts[i].nics[0].txbufs->pbuf[42]);
    }
    fio_mac_init_time_mgr(&sysconfig);
    if (fio_standby_ss(i, alises, mmap_addr) || fio_standby_upsnd())
        goto err;

    return 0;
err:
    return -1;
}

int fio_init()
{
    int rxqueues[2], txqueues[2];
    uint32_t nr_rx_slots[2], nr_tx_slots[2];
    struct nmreq nmrs[2];
    int i;
    if (already_running(LOCKFILE))
    {
        perror("already runing!"); 
        exit (1);
    }

    OD( "struct pktudp size %"SZF, sizeof(struct pktudp));
    OD( "struct arp pkt size %"SZF, sizeof(struct arp_packet));

    if ( 0 != fio_route_tbl_init() && sysconfig.single_thread != FIO_MODE_1_SND_OR_RCV)
        goto err;

    if (!sysconfig.is_test)
        sysconfig.pkt_size = 60;

    if (sysconfig.pkt_size < 16 || sysconfig.pkt_size > MAX_PKT_SIZE) 
    {
        OD( "bad pktsize %d\n", sysconfig.pkt_size);
        assert(sysconfig.pkt_size >= 16 || sysconfig.pkt_size <= MAX_PKT_SIZE);
    }

    //打印网卡信息
    if ( fio_nic_info(&nmrs[0], sysconfig.nic_names[0]) != 0)
        goto err;
    rxqueues[0] = nmrs[0].nr_rx_rings;
    txqueues[0] = nmrs[0].nr_tx_rings;
    nr_rx_slots[0] = nmrs[0].nr_rx_slots;
    nr_tx_slots[0] = nmrs[0].nr_tx_slots;
    OD( "if %s rxq %d txq %d rxsl %u txsl %u", 
            sysconfig.nic_names[0], rxqueues[0], txqueues[0], nr_rx_slots[0],
            nr_tx_slots[0]);

    if (fio_nic_info(&nmrs[1], sysconfig.nic_names[1]) != 0)
        goto err;
    rxqueues[1] = nmrs[1].nr_rx_rings;
    txqueues[1] = nmrs[1].nr_tx_rings;
    nr_rx_slots[1] = nmrs[1].nr_rx_slots;
    nr_tx_slots[1] = nmrs[1].nr_tx_slots;
    OD( "if %s rxq %d txq %d rxsl %u txsl %u", 
            sysconfig.nic_names[1], rxqueues[1], txqueues[1], nr_rx_slots[1],
            nr_tx_slots[1]);

    switch (sysconfig.single_thread)
    {
        case FIO_MODE_N_BRG_IN:
        case FIO_MODE_2CARDS:
        case FIO_MODE_N_BRG_OUT:
            if (sysconfig.num_nic > 1 && 
                    (rxqueues[0] != rxqueues[1] || 
                    txqueues[0] != txqueues[1] ||
                    nr_rx_slots[0] != nr_rx_slots[1] || 
                    nr_tx_slots[0] != nr_tx_slots[1]) )
            {
                OD( "rxqueues0 %s rxqueues1, txqueues0 %s txqueues1, "
                        "nr_rx_slots0 %s nr_rx_slots1, nr_tx_slots0 %s nr_tx_slots1",
                        rxqueues[0] != rxqueues[1] ? "!=" : "==",
                        txqueues[0] != txqueues[1] ? "!=" : "==",
                        nr_rx_slots[0] != nr_rx_slots[1] ? "!=" : "==",
                        nr_tx_slots[0] != nr_tx_slots[1] ? "!=" : "==");
                goto err;
            }
            sysconfig.nthreads = rxqueues[0];

            break;

        case FIO_MODE_1_SND_AND_RCV:
            sysconfig.nthreads = 1;
            OD( "one send/recv");
            break;
        case FIO_MODE_TEST_SND:
            sysconfig.nthreads = sysconfig.num_nic;
            OD( "test send nthreads %d", sysconfig.nthreads);
            break;
        case FIO_MODE_BRG_SINGLE:
            sysconfig.nthreads = 1;
            OD( "%d thread bridge in", sysconfig.nthreads);
            break;
        case FIO_MODE_1_SND_OR_RCV:
            sysconfig.nthreads = 2;
            OD( "one send, one recv");
            break;
        case FIO_MODE_1CPY_2SND_N_PROC:
            sysconfig.nthreads = 2;
            OD( "one recv, one snd, many process");
            break;

        default:
            OD( "has not implemented");
            goto err;
            break;
    }
    //end 打印网卡信息

    fio_log_init(&sysconfig.maclog, sysconfig.str_logpath, sysconfig.str_logfname);

    OD( "it will start %d threads", sysconfig.nthreads);
    sysconfig.burst = nr_rx_slots[0];
    OD( "burst is %d", sysconfig.burst);

    /* Install ^C handler. */
    //signal(SIGINT, fio_sigint_h);

    for (i = 0; i < FIO_MAX_NAME_LEN && sysconfig.interested_protos[i]; i++)
    {
        unsigned long _val = 1;
        fio_map_set_cpy(g_interested_tbl, sysconfig.interested_protos[i], &_val);
        fio_map_set_cpy(g_def_handles, sysconfig.def_protos[i], &_val);
    }

    fio_slog_open(inet_addr("192.168.23.167"), "dns126", "nfio", LOG_INFO, "local1");

    switch (sysconfig.single_thread)
    {
        case FIO_MODE_N_BRG_IN:
        case FIO_MODE_1_SND_AND_RCV:
        case FIO_MODE_2CARDS:
        case FIO_MODE_N_BRG_OUT:
        case FIO_MODE_BRG_SINGLE:
            if (fio_enter_mode0(txqueues) != 0)
                goto err;
            break;
        case FIO_MODE_1_SND_OR_RCV:
            if (fio_enter_mode2(txqueues) != 0)
                goto err;
            break;
        case FIO_MODE_1CPY_2SND_N_PROC:
            //if (fio_enter_1snd_1rcv_nproc(txqueues) != 0)
            //    goto err;
            break;
        case FIO_MODE_TEST_SND:
            if (fio_enter_mode3(txqueues) != 0)
                goto err;
            break;
        default:
            goto err;
            break;
    }
    //ERR(g_log, "context:%p nq:%p start:%d end:%d rs:%d is:%d buf:%p", 
    //        g_contexts, &g_contexts->notify_q_, g_contexts->notify_q_.__index_start,
    //        g_contexts->notify_q_.__index_end, g_contexts->notify_q_.__ring_size,
    //        g_contexts->notify_q_.__item_size, g_contexts->notify_q_.__buff);
    return 0;
err:
    OD( "error");
    return -1;
}

int fio_start()
{
    sysconfig.working = 1;
    if (fio_stack_start() < 0)
        goto start_err;

    int i;
    for (i = 0; i < sysconfig.nthreads; i++) 
    {
        switch (sysconfig.single_thread)
        {
            case FIO_MODE_1_SND_AND_RCV:
            case FIO_MODE_N_BRG_OUT:
                if (pthread_create(&g_contexts[i].thread, NULL, fio_bridge, &g_contexts[i]) == -1)
                {
                    OD( "Unable to create thread %d", i);
                    g_contexts[i].used = 0;
                    goto start_err;
                }
                break;
            case FIO_MODE_TEST_SND:
                if (pthread_create(&g_contexts[i].thread, NULL, fio_test_send, &g_contexts[i]) == -1)
                {
                    OD( "Unable to create thread %d", i);
                    g_contexts[i].used = 0;
                    goto start_err;
                }
                break;
            case FIO_MODE_1_SND_OR_RCV:
                if (pthread_create(&g_contexts[i].thread, NULL, fio_dns_test, &g_contexts[i]) == -1)
                {
                    OD( "Unable to create thread %d", i);
                    g_contexts[i].used = 0;
                    goto start_err;
                }
                break;
            case FIO_MODE_2CARDS:
                if (pthread_create(&g_contexts[i].thread, NULL, fio_2cards, &g_contexts[i]) == -1)
                {
                    OD( "Unable to create thread %d", i);
                    g_contexts[i].used = 0;
                    goto start_err;
                }
                break;
            case FIO_MODE_N_BRG_IN:
                if (pthread_create(&g_contexts[i].thread, NULL, fio_bridge_in_, &g_contexts[i]) == -1) 
                {
                    OD( "Unable to create thread %d", i);
                    g_contexts[i].used = 0;
                    goto start_err;
                }
                break;
            case FIO_MODE_BRG_SINGLE:
                if (pthread_create(&g_contexts[i].thread, NULL, fio_brg_single, &g_contexts[i]) == -1) 
                {
                    OD( "Unable to create thread %d", i);
                    g_contexts[i].used = 0;
                    goto start_err;
                }
                break;
            case FIO_MODE_1CPY_2SND_N_PROC:
                //if (pthread_create(&g_contexts[i].thread, NULL, , &g_contexts[i]) == -1)
                //{
                //    D("Unable to create thread %d", i);
                //    g_contexts[i].used = 0;
                //    goto start_err;
                //}
                break;
            default:
                OD( "has not implemented");
                goto start_err;
                break;
        }

        {
            cpu_set_t mask;
            cpu_set_t get;

            CPU_ZERO(&mask);
            CPU_SET(i, &mask);

            if (pthread_setaffinity_np(g_contexts[i].thread, sizeof(mask), &mask) < 0) 
            {
                OD( "tid %d set thread affinity failed", g_contexts[i].me);
            }
            CPU_ZERO(&get);
            if (pthread_getaffinity_np(g_contexts[i].thread, sizeof(get), &get) < 0) 
            {
                OD( "tid %d get thread affinity failed", g_contexts[i].me);
            }
            int j;
            for (j = 0; j < sysconfig.nthreads; j++) 
            {
                if (CPU_ISSET(j, &get)) 
                {
                    OD( "tid %d is running in processor %d", g_contexts[i].me, j);
                }
            }
        }
    }
    fio_open_timer(10);
    return 0;
start_err:
    return -1;
}

int fio_wait()
{
    int i;
    static int called = 0;
    if (called)
        return 0;

    called = 1;
    if (g_started_count)
        pthread_join(g_count_id, NULL);

    for (i = 0; i < sysconfig.nthreads; i++)
    {
        /*
         * Join active threads, unregister interfaces and close
         * file descriptors.
         */
        if (g_contexts[i].thread != 0)
            pthread_join(g_contexts[i].thread, NULL);
        //ioctl(g_contexts[i].fd, NIOCUNREGIF, &g_contexts[i].nmr);
        //close(g_contexts[i].fd);
    }

    return 0;
}

int fio_shutdown()
{
    static int shuted = 0;
    if (shuted)
        return 0;
    shuted = 1;

    int i, j;
    if (sysconfig.enable_sysstack && g_syscontext)
    {
        for (j = 0; j < g_syscontext->num_nic; j++)
        {
            fio_nic_close(&g_syscontext->nics[j]);
        }
    }

    for (i = 0; i < sysconfig.nthreads; i++) 
    {
        for (j = 0; j < g_contexts[i].num_nic; j++)
        {
            fio_nic_close(&g_contexts[i].nics[j]);
	    if (sysconfig.single_thread != FIO_MODE_TEST_SND)
	    {
            fio_mp_nic_destroy(&g_contexts[i].nics[j].mp_ipinfo);
            fio_mac_pktcache_list_destroy(g_contexts[i].nics[j].buddy_mac_pkt_factory, g_contexts[i].nics[j].pktcache_list_nic);
	    }
        }
        if (g_contexts[i].notifyfds)
            free(g_contexts[i].notifyfds);
        if (g_contexts[i].pktcache_list_nic)
            free(g_contexts[i].pktcache_list_nic);
	if (sysconfig.single_thread != FIO_MODE_TEST_SND)
	{
        fio_mac_destroy_buddy_mac(&g_contexts[i]);
        fio_mac_pkt_factory_destroy(&g_contexts[i].buddy_mac_pkt_factory);
        fio_route_defroute_clean(&g_contexts[i].defroute_context);
        fio_route_context_clean(&g_contexts[i].route_context);
	}
    }

    free(g_contexts);
    if (sysconfig.enable_sysstack && g_syscontext)
    {
        if (g_syscontext->notifyfds)
            free(g_syscontext->notifyfds);
        free(g_syscontext);
    }
    fio_slog_close();
    translate_port_clear();
    fio_destroy_cbf_tbl();
    fio_mp_destroy();
    fio_route_tbl_destroy();
    return 0;
}

int fio_start_statistics()
{
    if (g_started_count)
        return 0;

    pthread_create(&g_count_id, NULL, fio_count, &sysconfig);
    g_started_count = 1;

    return 0;
}

__attribute__((constructor))
static void _def_init()
{
    init_nm_util(); 
    fio_create_cbf_tbl();
}

