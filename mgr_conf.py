#!/usr/bin/python
# -*- coding:UTF-8 -*-

log_level='info'
log_path = '/usr/local/dnspro/log'
log_name = 'dnspro.log'
log_direction=1

g_inner_chk_init_ok_time = 2
g_inner_chk_snd_time = 10
g_inner_chk_task_domain_time = 5
g_inner_chk_task_record_time = 2

g_url_inner_chk_task_ip = '121.201.12.60'
g_url_inner_chk_task_url = 'http://' + g_url_inner_chk_task_ip + '/dnspro/dnsbroker/'
g_mgr_sid = 'node_mgr_primary_zs_bgp'

g_db_ip= '121.201.12.57'
g_db_user= 'root'
g_db_passwd = 'rjkj@rjkj'
g_db_db = 'dnspro_core'

g_zkClis = '121.201.12.57:2181,121.201.12.58:2181,121.201.12.60:2181,121.201.12.61:2181,121.201.12.62:2181'
#g_zkClis = 'localhost:2181'
