/*!
 * \file libbase_netutil.h
 * \brief 与网络相关的接口
 * \date 2012-11-07 08:53:30
 */

#ifndef _LIBBASE_NET_UTIL_H_
#define _LIBBASE_NET_UTIL_H_

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
//#include <linux/if.h>
#include <net/if.h>

#include <netinet/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "libbase_strutils.h"
#include "libbase_check_byteswap.h"

__BEGIN_DECLS


/**
* \brief 返回网口的mac地址，返回值为uint64_t, 是一个大端值
         如: mac:66:55:44:33:22:11 
         返回值为: 0x665544332211
         内存为:  [0]:66 [1]:55 [2]:44 [3]:33 [4]:22 [5]:11 
* @param   ifname   网口名
*
* @return 
* @remark null
* @see     
* @author lwj      @date 2012/12/05
**/
static inline uint64_t wns_get_ifmac(const char* ifname)
{
    int hw_socket = -1;
    int ret = 0;
    uint64_t ret_mac = 0;
    struct ifreq ifr_mac;

    hw_socket = socket(PF_INET, SOCK_STREAM, 0);
    if( hw_socket < 0)
    {
        fprintf(stderr,"check socket create\n");
        goto error;
    }
    memset(&ifr_mac, 0, sizeof(ifr_mac));
    strcpy_n(ifr_mac.ifr_name, sizeof(ifr_mac.ifr_name), ifname);

    ret = ioctl(hw_socket, SIOCGIFHWADDR, &ifr_mac);
    if( ret < 0)
    {
        fprintf(stderr,"check ioctl cmd\n");
        goto error;
    }

    memcpy(&ret_mac, ifr_mac.ifr_hwaddr.sa_data, sizeof(ret_mac));
#if	__BYTE_ORDER == __LITTLE_ENDIAN
    ret_mac = bswap_64(ret_mac);
#endif
    close(hw_socket);
    return ret_mac;
error:
    if (hw_socket != -1)
    {
        close(hw_socket);
    }
    return 0;
}


__END_DECLS

#endif /* end of include guard: _LIBWNS_APPNAME_ */

