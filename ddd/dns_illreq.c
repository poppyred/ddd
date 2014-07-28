#include "dns_illreq.h"
#include "fio_nic.h"


extern time_t global_now;

char g_ethname[512] = {0};
struct ether_addr *g_set_mac = NULL;
time_t g_timeout = 0;




struct ether_addr *dns_illreq_get_mac()
{
        return g_set_mac;
}



int dns_illreq_set(char *ethname, int ip)
{
    struct fio_nic * nic = fio_nic_fromip(ethname);
    if (nic)
    {
        g_set_mac = fio_mac_byip(nic,ip);
        if (g_set_mac)
        {
            return GET_MAC_SUCESS;
        }
           
    }

    if (g_timeout == 0)
    {
        g_timeout = global_now + 60;
    }
    else
    {
        if (global_now - g_timeout >= 0)
        {
            return GET_MAC_SUCESS;
        }
          
    }

    return GET_MAC_FAILED;
}


