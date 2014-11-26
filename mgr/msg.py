#!/usr/bin/python
# -*- coding:UTF-8 -*-
# made by likunxiang

g_isdebug = 1
g_enable_stdin=0
old_settings = None
g_websvr_ip = '121.201.11.4'
g_now = 0
g_log_maxline = 90000

g_init_resp_expect = -1
#g_init_complete = False

g_a_tbl = 1
g_ns_tbl = 2
g_cname_tbl = 5
g_mx_tbl = 15
g_txt_tbl = 16
g_aaaa_tbl = 28
g_any_tbl = 255

g_list_tbl = ('a_record', 'aaaa_record', 'cname_record', 'mx_record', 'ns_record', 'txt_record')
#g_dict_tbl_type = {'a_record':g_a_tbl,
#             'aaaa_record':g_ns_tbl,
#             'cname_record':g_cname_tbl,
#             'mx_record':g_mx_tbl,
#             'ns_record':g_ns_tbl,
#             'txt_record':g_txt_tbl}

g_dict_type = {'a_record':g_a_tbl, #'A':1, #指定计算机IP地址
        'ns_record':g_ns_tbl, #'NS':2, #指定用于命名区域的DNS名称服务器
        'MD':3, #指定邮件接收站(此类型已经过时了,使用MX代替)
        'MF':4, #指定邮件中转站(此类型已经过时了,使用MX代替)
        'cname_record':g_cname_tbl, #'CNAME':5, #指定用于别名的规范名称
        'SOA':6, #指定用于DNS区域的"起始授权机构"
        'MB':7, #指定邮箱域名
        'MG':8, #指定邮件组成员
        'MR':9, #指定邮件重命名域名
        'NULL':10, #指定空的资源记录
        'WKS':11, #描述已知服务
        'PTR':12, #如果查询是IP地址,则指定计算机名;否则指定指向其它信息的指针
        'HINFO':13, #指定计算机CPU以及操作系统类型
        'MINFO':14, #指定邮箱或邮件列表信息
        'mx_record':g_mx_tbl, #'MX':15, #指定邮件交换器
        'txt_record':g_txt_tbl, #'TXT':16, #指定文本信息
        'aaaa_record':g_aaaa_tbl, #指定aaaa
        'UINFO':100, #指定用户信息
        'UID':101, #指定用户标识符
        'GID':102, #指定组名的组标识符
        'ANY':255 #指定所有数据类型
        }

g_class_init = 'init'
g_class_init_test = 'init_test'
g_class_proxy_register = 'register'
g_class_proxy_heartbeat = 'heartbeat'
g_class_init_view_reply = 'view_reply'
g_class_init_dns_reply = 'dns_reply'

g_class_inner_chk_init_ok = 'chk_init_ok'
g_class_inner_chk_snd = 'chk_snd'
g_class_inner_chk_task_domain = 'chk_task_domain'
g_class_inner_chk_task_domain_reply = 'chk_task_domain_reply'
g_class_inner_chk_task_record = 'chk_task_record'
g_class_inner_chk_task_record_reply = 'chk_task_record_reply'
g_class_inner_chk_task_done = 'chk_task_done'
g_class_inner_chk_task_db_heartbeat = 'chk_task_db_heartbeat'

g_class_inner_reqtype_map = {g_class_inner_chk_task_domain:'domain', g_class_inner_chk_task_record:'record'}
g_class_inner_map = {g_class_inner_chk_task_domain:g_class_inner_chk_task_domain_reply,
        g_class_inner_chk_task_record:g_class_inner_chk_task_record_reply}

g_init_sql_view = 'SELECT viewid as view, network as mask from view_mask \
        mk left join view_index idx on mk.viewid=idx.id where mk.status=\'true\' '
g_init_sql_dns = 'SELECT ar.name as domain, ze.view as view FROM %s ar \
        left join zone ze on ar.zone=ze.id where ar.enable=1'
g_init_sql_chk_init_ok = 'SELECT COUNT(*) FROM snd_record WHERE state!=0'

g_proc_add_snd_req = 'add_snd_req'
#g_proc_add_snd_req_ret = 'select @_' + g_proc_add_snd_req + '_6'
g_proc_update_snd_req = 'update_snd_req'
#g_proc_update_snd_req_ret = 'select @_' + g_proc_update_snd_req + '_5'
g_proc_add_a_record = 'add_a_record'
g_proc_add_mx_record = 'add_mx_record'
g_proc_del_a_record = 'del_a_record'
g_proc_del_a_domain = 'del_a_domain'
g_proc_set_a_domain = 'onoff_a_domain'
g_proc_get_subrecord_inline = 'get_subrecord_inline'
g_sql_clean_snd_req = 'DELETE FROM `snd_record`'
#g_sql_get_a_record_in_set = 'SELECT ar.name,ze.view FROM a_record ar LEFT JOIN zone ze \
#        ON ar.`zone`=ze.`id` WHERE ar.`enable`=1 AND ar.rid=%s'
g_sql_add_a_domain_ns = 'INSERT INTO domain_ns(domain,ttl,`server`,`rid`)VALUES(\'%s\',%d,\'%s\',%d) \
        ON DUPLICATE KEY UPDATE domain=\'%s\',ttl=%d,`server`=\'%s\''
g_sql_add_a_view = 'INSERT INTO `view_index` (`id`,`view_name`,`comment`,`ttl`,`status`) VALUES \
        (%d,\'%s\',\'%s\',%d,\'true\') on DUPLICATE KEY UPDATE `comment`=\'%s\',`ttl`=%d,`status`=\'true\''
g_sql_del_a_view = 'DELETE FROM `view_index` WHERE `id`=%d'
g_sql_add_a_mask = 'insert into `view_mask` (`network`,`viewid`,`status`) values (\'%s\',%d,\'true\') \
        ON DUPLICATE KEY UPDATE `viewid`=%d,`status`=\'true\''
g_sql_del_a_mask = 'DELETE FROM `view_mask` WHERE `network`=\'%s\' and `viewid`=%d'

g_inner_sql_chksnd_view = 'SELECT viewid,`data`,`opt`,chktime FROM snd_record WHERE state=0 \
        and class=\'view\' AND chktime<=DATE_ADD(NOW(),INTERVAL -9 SECOND) \
        ORDER BY `id` ASC'
g_inner_sql_chksnd_dns = 'SELECT `type`,viewid,`data`,`opt`,chktime FROM snd_record WHERE state=0 \
        and class=\'dns\' AND chktime<=DATE_ADD(NOW(),INTERVAL -9 SECOND) \
        ORDER BY `id` ASC'

g_inner_sql_db_heartbeat = 'SELECT 1'

g_pack_head_init_view = 5
g_pack_head_init_dns = 4

g_opt_add = 1
g_opt_del = 2

g_http_response_OK = 200

