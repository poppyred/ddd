#!/usr/bin/python
# -*- coding:UTF-8 -*-

log_level='care'
log_path = '/usr/local/dnspro/log'
log_name = 'dnspro.log'
log_direction=1

g_size_perpack = 1024
g_row_perpack = 10
g_row_perpack4init = 500

g_listen_port = 54321
g_reply_port = 12345
g_heart_port = 54321

g_inner_chk_init_ok_time = 2
g_inner_chk_snd_time = 10
g_inner_chk_task_domain_time = 5
g_inner_chk_task_record_time = 2
g_inner_chk_task_db_heartbeat = 10

g_url_inner_chk_task_ip = 'dnspro-database'
g_url_inner_chk_task_url = 'http://' + g_url_inner_chk_task_ip + '/dnspro/dnsbroker/'
g_mgr_sid = 'node_mgr_primary_zs_bgp'

g_db_ip= 'localhost'
g_db_user= 'root'
g_db_passwd = 'rjkj@rjkj'
g_db_db = 'dnspro_core'

g_zkClis = '121.201.12.58:2181,121.201.12.60:2181,121.201.12.68:2181'
#g_zkClis = 'localhost:2181'
