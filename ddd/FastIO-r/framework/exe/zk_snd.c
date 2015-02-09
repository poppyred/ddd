//	$Id: fio.c 2013-05-23 likunxiang$
//
#include "fio.h"
#include "fio_test_send.h"
#include "fio_pack_misc.h"
#include <assert.h>

int log_pack_head(char *_result, const char *_module,int _len, const char  _log[][255], int *_llen, int _log_num)
{
    if (!_result || !_module)
    {
        D("error");
        return 0;
    }


    char *begin = _result;
    _result[0] = _len;
    _result += 1;
    memcpy(_result, _module, _len);
    _result += _len;

    int i;
    for (i = 0; i < _log_num; i++)
    {
        *(uint16_t*)(_result) = htons(_llen[i]);
        _result += 2;
        memcpy(_result, _log[i], _llen[i]);
        _result += _llen[i];
    }

    return _result - begin;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("argc < 2\n");
        return 0;
    }

    if (0 != parse_config(argv[1]))
        return 0;
    if (0 != fio_init())
        return 0;

    int j;
    struct fio_txdata txds[FIO_MAX_NICS];
    char buf_if_necessary[2][MAX_PKT_SIZE];
    struct pktudp *pkt4test[] = {&g_pkt4test, &g_pkt4test2};

    char modules[][255] = {
        "DNS"
    };
    int module_len[] = {
        strlen("DNS")
    };

    char logs[][255] = {
        "agasgjsad;gasgasghasgasfgafsagadasgjklasg;lsjkl;\n"
    };

    int log_len[] = {
        strlen("agasgjsad;gasgasghasgasfgafsagadasgjklasg;lsjkl;\n")
    };


    for (j = 0; j < sysconfig.num_nic; j++)
    {
        struct fio_txdata *txd = &txds[j];
        memset(txd, 0, sizeof(struct fio_txdata));
        txd->type = TXDATA_TYPE_NORMAL;
        txd->pbuf = buf_if_necessary[j];
        txd->pdata = txd->pbuf+g_payload_offset;
        memcpy(txd->pbuf, (char*)pkt4test[j], sysconfig.pkt_size);
        int paylen = log_pack_head(txd->pdata, modules[j], module_len[j], (const char (*)[255])logs, log_len, 1);
        txd->size = g_payload_offset+paylen;
        *(uint16_t*)(txd->pbuf+38) = htons(paylen+8);
        *(uint16_t*)(txd->pbuf+16) = htons(paylen+8+20);

        struct pktudp *ppkt = (struct pktudp*)txd->pbuf;
        *(uint32_t*)(txd->pbuf+g_sip_offset) = sysconfig.src_ip.start.s_addr;
        bcopy(&sysconfig.src_mac.start, txd->pbuf+g_eth_smac, 6);
        ppkt->udp.uh_sum = 0;

        struct ip *ip = &ppkt->ip;                                                 
        ip->ip_sum = 0;
        ip->ip_sum = ip_fast_csum((char*)ip, ip->ip_hl);
    }

    fio_test_snd_set_pkt(txds);

    if (0 != fio_start())
        return 0;
    fio_start_statistics();

    fio_wait();
    fio_shutdown();

    return 0;
}

