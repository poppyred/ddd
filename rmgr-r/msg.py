#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

g_isdebug = 1
g_enable_stdin = 0
old_settings = None
g_websvr_ip = '121.201.11.4'
g_now = 0
g_log_maxline = 90000
g_init_resp_expect = -1
g_init_maxid = 0
g_init_sendstate = 0 #0 uncare , 1 sending , 2 sent
g_a_tbl = 1
g_ns_tbl = 2
g_cname_tbl = 5
g_ptr_tbl = 12
g_mx_tbl = 15
g_txt_tbl = 16
g_aaaa_tbl = 28
g_any_tbl = 255
g_list_tbl = ('a_record', 'aaaa_record', 'cname_record', 'mx_record', 'ns_record', 'txt_record', 'ptr_record')
g_dict_type = {'a_record': g_a_tbl,
 'ns_record': g_ns_tbl,
 'MD': 3,
 'MF': 4,
 'cname_record': g_cname_tbl,
 'SOA': 6,
 'MB': 7,
 'MG': 8,
 'MR': 9,
 'NULL': 10,
 'WKS': 11,
 'ptr_record': g_ptr_tbl,
 'HINFO': 13,
 'MINFO': 14,
 'mx_record': g_mx_tbl,
 'txt_record': g_txt_tbl,
 'aaaa_record': g_aaaa_tbl,
 'UINFO': 100,
 'UID': 101,
 'GID': 102,
 'ANY': 255}
g_class_init = 'init'
g_class_init_test = 'init_test'
g_class_proxy_register = 'register'
g_class_proxy_heartbeat = 'heartbeat'
g_class_init_view_reply = 'view_reply'
g_class_init_dns_reply = 'dns_reply'
g_class_inner_chk_init_ok = 'chk_init_ok'
g_class_inner_chk_snd = 'chk_snd'
g_class_inner_chk_init = 'chk_init'
g_class_inner_chk_task_domain = 'chk_task_domain'
g_class_inner_chk_task_domain_reply = 'chk_task_domain_reply'
g_class_inner_chk_task_record = 'chk_task_record'
g_class_inner_chk_task_record_reply = 'chk_task_record_reply'
g_class_inner_chk_task_done = 'chk_task_done'
g_class_inner_reqtype_map = {g_class_inner_chk_task_domain: 'domain', g_class_inner_chk_task_record: 'record'}
g_class_inner_map = {g_class_inner_chk_task_domain: g_class_inner_chk_task_domain_reply,
        g_class_inner_chk_task_record: g_class_inner_chk_task_record_reply}

g_init_sql_view = "SELECT viewid as view, network as mask from view_mask mk left join view_index idx on mk.viewid=idx.id where mk.status='true' "
g_init_sql_dns = 'SELECT distinct ar.name as domain, ze.view as view FROM %s ar left join zone ze on ar.zone=ze.id where ar.enable=1'
g_init_sql_chk_init_ok = 'SELECT COUNT(*) FROM snd_record WHERE state!=0'
g_init_sql_inittask_dns = 'SELECT `id`,`type`,viewid,`data` FROM snd_record WHERE state=0 and class=\'dns\' and `opt`=1 ORDER BY `id` ASC'
g_init_sql_inittask_dns_limit1 = 'SELECT `id`,`type`,viewid,`data` FROM snd_record WHERE state=0 and class=\'dns\' and `opt`=1 ORDER BY `id` ASC limit 1'
g_init_sql_inittask_dns_inited = 'update snd_record set state=4 WHERE `id`=%d'
g_init_sql_inittask_view = 'SELECT `id`,viewid,`data` FROM snd_record WHERE state=0 \
        and class=\'view\' AND `opt`=1 ORDER BY `id` ASC'
g_init_sql_inittask_view_inited = 'update snd_record set state=4 WHERE `id`=%d'
g_init_sql_replytask = 'update snd_record set state=%d WHERE `id`=%d'
g_init_sql_gettask_dns = 'SELECT `id`,`type`,viewid,`data`,`opt` FROM snd_record WHERE state=0 \
        and class=\'dns\' ORDER BY `id` ASC'
g_init_sql_get_name_view_zone_by_rid = 'SELECT ar.name,ze.view,ar.zone FROM %s ar LEFT JOIN zone ze ON ar.zone=ze.id \
        WHERE ar.enable=1 AND ar.rid=%d LIMIT 1';
g_init_sql_get_name_view_zone_by_rid_only = 'SELECT ar.name,ze.view,ar.zone FROM %s ar LEFT JOIN zone ze ON ar.zone=ze.id \
        WHERE ar.rid=%d LIMIT 1';
g_init_sql_count_by_name_zone = 'SELECT count(*) FROM %s WHERE `name`=\'%s\' AND `zone`=%d AND `enable`=1'
g_init_sql_update_snd_req_dns = 'update snd_record set `state`=1 WHERE `class`=\'dns\' AND `type`=%d AND \
        `viewid`=%d and `data`=\'%s\' and `opt`=%d'
g_init_sql_gettask_mask = 'SELECT `id`,viewid,`data`,`opt` FROM snd_record WHERE state=0 \
        and class=\'view\' ORDER BY `id` ASC'
g_init_sql_counttask = 'SELECT count(*), max(id) FROM `snd_record` where class=\'view\' or class=\'dns\''

g_proc_add_snd_req = 'add_snd_req'
g_proc_update_snd_req = 'update_snd_req'
g_proc_add_a_record = 'add_a_record'
g_proc_add_ptr_record = 'add_ptr_record'
g_proc_add_mx_record = 'add_mx_record'
g_proc_del_a_record = 'del_a_record'
g_proc_del_a_domain = 'del_a_domain'
g_proc_set_a_domain = 'onoff_a_domain'
g_proc_get_subrecord_inline = 'get_subrecord_inline'
g_proc_add_task = 'add_snd_req_new'

g_sql_clean_snd_req = 'DELETE FROM `snd_record`'
g_sql_add_a_domain_ns = "INSERT INTO domain_ns(domain,ttl,`server`,`rid`)VALUES('%s',%d,'%s',%d) \
        ON DUPLICATE KEY UPDATE domain='%s',ttl=%d,`server`='%s'"
g_sql_add_snd_req = 'INSERT INTO `snd_record` (class,`type`,viewid,`data`,state,`opt`) \
        VALUES(\'%s\',%d,%d,\'%s\',%d,%d)'
#g_sql_get_exist_records = 'SELECT DISTINCT ze.domain FROM a_record ar LEFT JOIN zone ze ON ar.zone = ze.id \
#        WHERE ze.domain = \'lkx.cc\' and ar.enable=1'
g_sql_get_exist_records = 'SELECT DISTINCT ze.domain FROM `%s` ar LEFT JOIN zone ze ON ar.zone = ze.id \
        WHERE ze.domain = \'%s\' and ar.enable=1'

g_inner_sql_chksnd_view = 'SELECT `id`,viewid,`data`,`opt`,chktime FROM snd_record WHERE state=4 \
        and class=\'view\' AND chktime<=DATE_ADD(NOW(),INTERVAL -20 SECOND) ORDER BY `id` ASC'
g_inner_sql_chksnd_dns = 'SELECT `id`,`type`,viewid,`data`,`opt`,chktime FROM snd_record WHERE state=4 \
        and class=\'dns\' AND chktime<=DATE_ADD(NOW(),INTERVAL -20 SECOND) ORDER BY `id` ASC'

g_inner_sql_db_heartbeat = 'SELECT 1'
g_pack_head_init_view = 5
g_pack_head_init_dns = 4
g_pack_head_init_complete = 7
g_opt_add = 1
g_opt_del = 2
g_http_response_OK = 200
