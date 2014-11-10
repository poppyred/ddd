//	$Id: proto_map.c 2013-05-23 likunxiang$
//
#include "proto_map.h"
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

struct name2proto
{
	char *name; 
	uint8_t proto;
};

struct name2proto name2proto[MAX_PROTO+1] = {
    {"ip", IPPROTO_IP},
    {"icmp", IPPROTO_ICMP},
    {"igmp", IPPROTO_IGMP},
    {"ipip", IPPROTO_IPIP},
    {"tcp", IPPROTO_TCP},
    {"egp", IPPROTO_EGP},
    {"pup", IPPROTO_PUP},
    {"udp", IPPROTO_UDP},
    {"idp", IPPROTO_IDP},
    {"tp", IPPROTO_TP},
    {"dccp", IPPROTO_DCCP},
    {"ipv6", IPPROTO_IPV6},
    {"routing", IPPROTO_ROUTING},
    {"fragment", IPPROTO_FRAGMENT},
    {"rsvp", IPPROTO_RSVP},
    {"gre", IPPROTO_GRE},
    {"esp", IPPROTO_ESP},
    {"ah", IPPROTO_AH},
    {"icmpv6", IPPROTO_ICMPV6},
    {"none", IPPROTO_NONE},
    {"dstops", IPPROTO_DSTOPTS},
    {"mtp", IPPROTO_MTP},
    {"encap", IPPROTO_ENCAP},
    {"pim", IPPROTO_PIM},
    {"comp", IPPROTO_COMP},
    {"sctp", IPPROTO_SCTP},
    {"udplite", IPPROTO_UDPLITE},
    {"raw", IPPROTO_RAW},
	{NULL, 0},
};

int proto_str2int(char * name)
{
    int j;
    for (j = 0; name2proto[j].name; j++)
        if (!strcmp(name2proto[j].name, name))
            return name2proto[j].proto;
    return -1;
}

