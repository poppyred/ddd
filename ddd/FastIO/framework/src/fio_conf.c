//	$Id: fio_conf.c 2013-05-23 likunxiang$
//
#include "fio_conf.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "fio_sysconfig.h"
#include "log_log.h"

struct IPMAC * g_ip_macs[MAX_IPMAC];
int g_mac_num = 0;
static int g_struct = 0;

enum
{
    FM_INT,
    FM_STR,
    FM_IPMAC,
    FM_DST,
};

#define   FML_SINGLE ((uint32_t)0x1)
#define   FML_STRUCT ((uint32_t)0x2)

struct fetch_method
{
    const char *key_name;
    int fm;
    uint32_t flag;
    void *dst;
};

struct fetch_method g_fms[] = {
    {"src_ip",                  FM_STR,         FML_SINGLE,        sysconfig.src_ip.name},
    //{"dst_ip",                  FM_STR,         FML_SINGLE,        sysconfig.dst_ip[0].name},
    //{"dst_ip2",                 FM_STR,         FML_SINGLE,        sysconfig.dst_ip[1].name},
    {"src_mac",                 FM_STR,         FML_SINGLE,        sysconfig.src_mac.name},
    //{"dst_mac",                 FM_STR,         FML_SINGLE,        sysconfig.dst_mac[0].name},
    //{"dst_mac2",                FM_STR,         FML_SINGLE,        sysconfig.dst_mac[1].name},
    {"src_port",                FM_INT,         FML_SINGLE,        &sysconfig.src_port},
    {"dst_port",                FM_INT,         FML_SINGLE,        &sysconfig.dst_port},
    {"pkt_size",                FM_INT,         FML_SINGLE,        &sysconfig.pkt_size},
    {"prdebug",                 FM_INT,         FML_SINGLE,        &sysconfig.prdebug},
    {"prmac_info",              FM_INT,         FML_SINGLE,        &sysconfig.prmac_info},
    {"needcp",                  FM_INT,         FML_SINGLE,        &sysconfig.needcp},
    {"if_in",                   FM_STR,         FML_SINGLE,        sysconfig.if_in},
    {"if_out",                  FM_STR,         FML_SINGLE,        sysconfig.if_out},
    {"is_test",                 FM_INT,         FML_SINGLE,        &sysconfig.is_test},
#ifdef __old_mac_mgr__
    {"vlan",                    FM_STR,         FML_SINGLE,        sysconfig.vlans[0]},
    {"vlan2",                   FM_STR,         FML_SINGLE,        sysconfig.vlans[1]},
#endif
    {"<ip_mac>",                FM_IPMAC,       FML_STRUCT,        NULL},
    {"mac_lifetime",            FM_INT,         FML_SINGLE,        &sysconfig.mac_lifetime},
    {"mac_try",                 FM_INT,         FML_SINGLE,        &sysconfig.mac_try},
    {"single_thread",           FM_INT,         FML_SINGLE,        &sysconfig.single_thread},
    {"interested_proto",        FM_STR,         FML_SINGLE,        sysconfig.str_interested_proto},
    {"interested_port",         FM_STR,         FML_SINGLE,        sysconfig.str_interested_port},
    {"check_route_interval",    FM_INT,         FML_SINGLE,        &sysconfig.route_chk_interval},
    {"snd_pkts_per_sec",        FM_INT,         FML_SINGLE,        &sysconfig.snd_pkts_per_sec},
    {"<dst>",                   FM_DST,         FML_STRUCT,        NULL},
    {"logpath",                 FM_STR,         FML_SINGLE,        sysconfig.str_logpath},
    {"logfname",                FM_STR,         FML_SINGLE,        sysconfig.str_logfname},
    {"burst",                   FM_INT,         FML_SINGLE,        &sysconfig.burst},
    {"def_proto",               FM_STR,         FML_SINGLE,        &sysconfig.str_def_proto},
    {"promisc",                 FM_INT,         FML_SINGLE,        &sysconfig.promisc},
    {"defgw_mac",               FM_STR,         FML_SINGLE,        sysconfig.defgw_mac.name},
    {"prot_defgw_mac",          FM_INT,         FML_SINGLE,        &sysconfig.prot_defgw_mac},
    {"enable_sysstack",         FM_INT,         FML_SINGLE,        &sysconfig.enable_sysstack},
    {"enable_slog",             FM_INT,         FML_SINGLE,        &sysconfig.enable_slog},
    {"buddymac_checktime",      FM_INT,         FML_SINGLE,        &sysconfig.mac_buddy_lifetime},
    {"spec_ip",                 FM_STR,         FML_SINGLE,        sysconfig.str_spec_ip},
    {"bufnum",                  FM_INT,         FML_SINGLE,        &sysconfig.buf_num_},
    {"LogFolder",               FM_STR,         FML_SINGLE,        sysconfig.log_folder_},
    {"prsys_stack",             FM_INT,         FML_SINGLE,        &sysconfig.prsys_stack},
    {NULL},
};

void GetArg(char* arg, int size, char **pp)
{
	char *p = NULL;
	char *q = NULL;
	int quote = 0;

	p = *pp;
	while(isspace(*p))
		p++;
	q = arg;
	if (*p == '\"' || *p == '\'')
		quote = *p++;
	while(1)
	{
		if (quote && quote == *p)
		{
			p++;
			break;
		}
		if (quote == 0 && (isspace(*p) || *p =='\0'))
			break;
		if ((q- arg) < (size - 1))
			*q++ = *p++;
	}
	*q = '\0';
	*pp = p;
}

void translate_proto(char **names, int len, int dowhat)
{
	int i, j, k;
	for (i = 0, j = 0; i < 256 && i < len; i++)
	{
        if (-1 != (k=proto_str2int(names[i])))
        {
            switch (dowhat)
            {
                case 1:
                    sysconfig.interested_protos[j++] = k;
                    break;
                case 2:
                    sysconfig.def_protos[j++] = k;
                    break;
            }
        }
        else
            OD("<WARN> proto %s unknown", names[i]);
	}
}

void translate_port_clear()
{
    struct port_range *ptr;
    struct port_range **port_ranges = sysconfig.interested_ports_range;
    for (; (ptr = *port_ranges); port_ranges++)
        free(ptr);
}

void translate_port(char **strs, int len, int pos)
{
    if (pos < 256 && len == 1)
    {
        sysconfig.interested_ports_range[pos] = calloc(1,sizeof(struct port_range));
		sysconfig.interested_ports_range[pos]->pr_st = (uint16_t)atoi(strs[0]);
		sysconfig.interested_ports_range[pos]->pr_ed = (uint16_t)atoi(strs[0]);
    }
    else if (pos < 256 && len > 1)
    {
        sysconfig.interested_ports_range[pos] = calloc(1,sizeof(struct port_range));
		sysconfig.interested_ports_range[pos]->pr_st = (uint16_t)atoi(strs[0]);
		sysconfig.interested_ports_range[pos]->pr_ed = (uint16_t)atoi(strs[1]);
    }
}

static int CheckValid()
{
	char temp_arr[1024];
	char *temp_strs[1024];
    int i, l, len_strs;
	char *port_strs[1024];
	char port_range[1024];
	char mybuf[] = "00:00:00:00:00:00";

	if (strlen(sysconfig.if_in) < 1 || strlen(sysconfig.if_out) < 1)
	{
        OD("missing ifnames");
		return -1;
	}

	if (sysconfig.is_test)
	{
		if (strlen(sysconfig.src_mac.name) < 1)
		{
			/* retrieve source mac address. */
			if (source_hwaddr(sysconfig.if_out, mybuf) == -1) {
                OD("Unable to retrieve source mac");
				return -1;
				// continue, fail later
			}
			strcpy(sysconfig.src_mac.name, mybuf);
		}
		/* extract address ranges */
		extract_ip_range(&sysconfig.src_ip);
		extract_mac_range(&sysconfig.src_mac);

        for (i = 0; i < sysconfig.dst_count && i < FIO_MAX_DST_TEST; i++)
        {
            extract_ip_range(&sysconfig.dst_ip[i]);
            extract_mac_range(&sysconfig.dst_mac[i]);
        }
	}

	if (source_hwaddr(sysconfig.if_in, mybuf) == -1) {
        OD("Unable to retrieve source mac %s", sysconfig.if_in);
		return -1;
	}
	bcopy(ether_aton(mybuf), &sysconfig.if_macs[0], ETH_ALEN);
    OD("if %s mac %s", sysconfig.if_in, mybuf);
	if (source_hwaddr(sysconfig.if_out, mybuf) == -1) {
        OD("Unable to retrieve source mac %s", sysconfig.if_out);
		return -1;
	}
	bcopy(ether_aton(mybuf), &sysconfig.if_macs[1], ETH_ALEN);
    OD("if %s mac %s", sysconfig.if_out, mybuf);

    extract_mac_range(&sysconfig.defgw_mac);
    OD("defgw mac %s", sysconfig.defgw_mac.name);

	if (get_if_ip(sysconfig.if_in, &sysconfig.if_ips[0]))
        OD("<error> %s get ip", sysconfig.if_in);
	if (get_if_ip(sysconfig.if_out, &sysconfig.if_ips[1]))
        OD("<error> %s get ip", sysconfig.if_out);

    OD("read %d ipmac", g_mac_num);

	len_strs = splite_str(sysconfig.str_interested_proto, 
			sizeof(sysconfig.str_interested_proto), temp_arr, 
			1024, temp_strs, 1024, ' ');
	translate_proto(temp_strs, len_strs, 1);

	len_strs = splite_str(sysconfig.str_def_proto, 
			sizeof(sysconfig.str_def_proto), temp_arr, 
			1024, temp_strs, 1024, ' ');
	translate_proto(temp_strs, len_strs, 2);

	len_strs = splite_str(sysconfig.str_interested_port, 
			sizeof(sysconfig.str_interested_port), temp_arr, 
			1024, temp_strs, 1024, ' ');

    for (i = 0; i < len_strs; i++)
    {
        l = splite_str(temp_strs[i], strlen(temp_strs[i])+1, 
                port_range, 1024, port_strs, 1024, '~');
        translate_port(port_strs, l, i);
    }
    sysconfig.mac_lifetime *= 1000000;
    sysconfig.mac_buddy_lifetime *= 1000000;
    //sysconfig.multip_checktime *= 1000000;

    if (strlen(sysconfig.if_in))
        strncpy(sysconfig.nic_names[sysconfig.num_nic++], sysconfig.if_in, FIO_MAX_NAME_LEN);
    if (strlen(sysconfig.if_out) && strcmp(sysconfig.if_out, sysconfig.if_in))
        strncpy(sysconfig.nic_names[sysconfig.num_nic++], sysconfig.if_out, FIO_MAX_NAME_LEN);

	return 0;
}

static int get_str(char *dst, char *value, int v_s, char *p)
{
	GetArg(value, v_s, &p);
	strcpy(dst, value);
	return 0;
}

static int get_int(int *dst, char *value, int v_s, char *p)
{
	GetArg(value, v_s, &p);
	*dst = atoi(value);
	return 0;
}

static struct IPMAC * get_ipmac(FILE *fp)
{
	char line_buf[1024] = {0};
	char key[64] = {0};
	char *p = NULL;
	char value[1024] = {0};
	int complete = 0;

	struct IPMAC *ptr = (struct IPMAC*)malloc(sizeof(struct IPMAC));
	if (!ptr)
	{
        OD("error, alloc ipmac error out of memory!");
		exit(0);
	}

	while(1)
    {
        if (fgets(line_buf, sizeof(line_buf), fp) == NULL)
            break;
        p = line_buf;
        while(isspace(*p))
            p++;
        if (*p == '\0' || *p == '#')
            continue;

        GetArg(key, sizeof(key), &p);

        if (!strcasecmp(key, "ip"))
            get_str(ptr->ip, value, sizeof(value), p);
        else if (!strcasecmp(key, "mac"))
            get_str(ptr->mac, value, sizeof(value), p);
        else if (!strcasecmp(key, "via"))
            get_str(ptr->via, value, sizeof(value), p);
        else if (!strcasecmp(key, "</ip_mac>"))
        {
            //在这里转换ip,mac
            inet_aton(ptr->ip, &ptr->_ip);
            bcopy(ether_aton(ptr->mac), &ptr->_mac, ETH_ALEN);
            complete = 1;
            break;
        }
    }

    if (!complete)
    {
        free(ptr);
        ptr = NULL;
    }
    return ptr; 
}

static struct IPDST * get_dst(FILE *fp, struct IPDST *ptr)
{
	char line_buf[1024] = {0};
	char key[64] = {0};
	char *p = NULL;
	char value[1024] = {0};
	int complete = 0;

	while(1)
    {
        if (fgets(line_buf, sizeof(line_buf), fp) == NULL)
            break;
        p = line_buf;
        while(isspace(*p))
            p++;
        if (*p == '\0' || *p == '#')
            continue;

        GetArg(key, sizeof(key), &p);

        if (!strcasecmp(key, "dst_ip"))
            get_str(ptr->ip, value, sizeof(value), p);
        else if (!strcasecmp(key, "dst_mac"))
            get_str(ptr->mac, value, sizeof(value), p);
        else if (!strcasecmp(key, "</dst>"))
        {
            complete = 1;
            break;
        }
    }

    if (!complete)
        return NULL;
    return ptr; 
}

int parse_config(const char *file_name)
{
    pid_t pid = getpid();                                                                                                                                                                             
    snprintf(sysconfig.error_path_, MAX_FIO_PATH, "/var/log/error_nfio_%d.log", pid); 
    snprintf(sysconfig.trace_path_, MAX_FIO_PATH, "/var/log/trace_nfio_%d.log", pid);                                                                                                                          
    FILE *fp;
    if ((fp = fopen(file_name, "r")) == NULL)
    {
        printf("fopen() failed\n");
        return -1;
    }

    char line_buf[1024] = {0};
    char key[64] = {0};
    char value[1024] = {0};
    char *p = NULL;
    int pos = 0, md = FM_STR;
    struct fetch_method *pfm;

    while(1)
    {
        if (fgets(line_buf, sizeof(line_buf), fp) == NULL)
            break;
        p = line_buf;
        while(isspace(*p))
            p++;
        if (*p == '\0' || *p == '#')
            continue;

        GetArg(key, sizeof(key), &p);

        for (pfm = g_fms; pfm->key_name; pfm++)
        {
            if (!strcasecmp(key, pfm->key_name))
            {
                md = pfm->fm;
                break;
            }
        }

        if (!pfm->key_name)
        {
            OD("%s doesn't been implemented!", key);
            continue;
        }

        if (pfm->flag&FML_STRUCT)
        {
            if (1 == g_struct)
            {
                OD("<error> Already in a struct");
                goto config_error;
            }
            else
                g_struct = 1;
        }

        switch (md)
        {
            case FM_STR:
                get_str((char*)pfm->dst, value, sizeof(value), p);
                break;
            case FM_INT:
                get_int((int*)pfm->dst, value, sizeof(value), p);
                break;
            case FM_IPMAC:
                {
                    struct IPMAC *ptr;
                    if (NULL != (ptr=get_ipmac(fp)))
                    {
                        g_struct = 0;
                        g_ip_macs[pos++] = ptr;
                        g_mac_num++;
                    }
                }
                break;
            case FM_DST:
                {
                    struct IPDST ipdst;
                    if (NULL != get_dst(fp, &ipdst))
                    {
                        if (sysconfig.dst_count + 1 > FIO_MAX_DST_TEST)
                        {
                            OD("<error> too many dst tags");
                            goto config_error;
                        }
                        strcpy(sysconfig.dst_ip[sysconfig.dst_count].name, ipdst.ip);
                        strcpy(sysconfig.dst_mac[sysconfig.dst_count].name, ipdst.mac);
                        sysconfig.dst_count++;
                        g_struct = 0;
                    }
                    else
                    {
                        OD("<error> get dst tags");
                        goto config_error;
                    }
                }
                break;
            default:
                break;
        }
    }

    fclose(fp);
	return CheckValid();

config_error:
    fclose(fp);
	 return -1;
}

