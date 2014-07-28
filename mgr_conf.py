#!/usr/bin/python
# -*- coding:UTF-8 -*-

log_level = 'debug'
log_path = '/usr/local/dnspro/log'
log_name = 'dnspro.log'
log_direction = 0

g_inner_chk_snd_time = 10
g_inner_chk_task_domain_time = 5
g_inner_chk_task_record_time = 2

g_url_inner_chk_task_ip = '121.201.11.6'
g_url_inner_chk_task_url = 'http://' + g_url_inner_chk_task_ip + '/dnspro/dnsbroker/'
g_mgr_sid = 'node_mgr_primary_zs_bgp'

g_db_ip= '121.201.11.3'
g_db_user= 'root'
g_db_passwd = 'qwer'
g_db_db = 'dnspro_core'

