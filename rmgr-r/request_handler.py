#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import MySQL
import msg
import sys
import json
import types
import mgr_conf
import urllib
import mgr_err_describe
import time
from mgr_misc import partition_reserve_ip_from_ptr
import sys

class req_handler(object):
    @staticmethod
    def notify_proxy(worker, msgobj, addr, flush = False):
        if not flush and len(msgobj) >= mgr_conf.g_row_perpack or flush and len(msgobj) > 0:
            pre_pkt_head = None
            pre_i = 0
            for i in range(len(msgobj)):
                if msgobj[i].has_key('pkt_head'):
                    cur_pkt_head = msgobj[i].pop('pkt_head')
                else:
                    print >> sys.stderr,  ('no pkt_head key-->' + repr(msgobj[i]))
                    cur_pkt_head = None
                if cur_pkt_head and pre_pkt_head and cur_pkt_head != pre_pkt_head:
                    #print >> sys.stderr,  ('pkt piece head:' + str(pre_pkt_head) + ' -->' + repr(msgobj[pre_i:i]))
                    worker.reply(msgobj[pre_i:i], pre_pkt_head, addr)
                    pre_i = i
                if cur_pkt_head:
                    pre_pkt_head = cur_pkt_head

            if pre_pkt_head == None:
                print >> sys.stderr,  ('no pkt_head key-->' + repr(msgobj))
            else:
                #print >> sys.stderr,  ('pkt piece head:' + str(pre_pkt_head) + ' -->' + repr(msgobj[pre_i:]))
                worker.reply(msgobj[pre_i:], pre_pkt_head, addr)
            del msgobj[:]

    @staticmethod
    def notify_flush(worker, msgobj):
        if len(worker.proxy_addr.keys()) < 1:
            return
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], True)

    @staticmethod
    def set_chk_init_timer(worker):
        worker.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_snd)
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_init, mgr_conf.g_inner_chk_init_time)
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_domain, mgr_conf.g_inner_chk_task_domain_time)
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_record, mgr_conf.g_inner_chk_task_record_time)
        print >> sys.stderr,  'set chk_init_time'

    @staticmethod
    def set_chk_snd_timer(worker):
        worker.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init)
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
        print >> sys.stderr,  'set chk_snd_time'

    @staticmethod
    def handle_proxy_init_new(worker, addr):
        msg.g_init_sendstate = 1
        worker.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_snd)
        worker.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init)
        worker.dbcon.query(msg.g_sql_clean_snd_req)

        msgobj = []
        count = 0
        cur_cnt = 0
        expect_cnt = 0

        #生成任务 record
        for atbl in msg.g_list_tbl:
            if worker.should_stop == 1: return
            worker.dbcon.query(msg.g_init_sql_dns % (atbl))
            result = worker.dbcon.show()
            if not result:
                continue
            for row in result:
                if worker.should_stop == 1: return
                worker.dbcon.call_proc(msg.g_proc_add_task, ('dns', msg.g_dict_type[atbl], row[1], row[0], 0, msg.g_opt_add))

        #生成任务 mask
        worker.dbcon.query(msg.g_init_sql_view)
        if worker.should_stop == 1: return
        result = worker.dbcon.show()
        if result:
            for row in result:
                if worker.should_stop == 1: return
                worker.dbcon.call_proc(msg.g_proc_add_task, ('view', 0, row[0], row[1], 0, msg.g_opt_add))

        #计算任务数量
        worker.dbcon.query(msg.g_init_sql_counttask)
        result = worker.dbcon.show()
        if result and int(result[0][0]) != 0:
            expect_cnt += int(result[0][0])
            msg.g_init_maxid = int(result[0][1])
        else:
            msg.g_init_sendstate = 2
            req_handler.set_chk_init_timer(worker)
            req_handler.set_chk_snd_timer(worker)
            print >> sys.stderr,  'count task error open all timer'
            return

        #发送通知
        if worker.should_stop == 1: return
        worker.dbcon.query(msg.g_init_sql_inittask_dns)
        result = worker.dbcon.show()
        if result:
            for row in result:
                if worker.should_stop == 1: return
                worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
                print >> sys.stderr,  'dns query %s res: %s' % (atbl, row)
                msgobj.append({'id':row[0],'opt':msg.g_opt_add, 'domain':row[3], 'view':row[2], 'type':row[1]})
                count += 1
                if msg.g_init_sendstate == 1 and expect_cnt-count < 10:
                    msg.g_init_sendstate = 2
                    req_handler.set_chk_init_timer(worker)
                cur_cnt += 1
                if cur_cnt >= mgr_conf.g_row_perpack4init:
                    if worker.sendto_(msgobj, addr, msg.g_pack_head_init_dns, mgr_conf.g_reply_port) != True:
                        return
                    cur_cnt = 0
                    del msgobj[:]
                    time.sleep(1)

        if cur_cnt > 0:
            if worker.sendto_(msgobj, addr, msg.g_pack_head_init_dns, mgr_conf.g_reply_port) != True:
                return
            time.sleep(1)
        print >> sys.stderr, ("sent %d records" % (count,));

        del msgobj[:]
        cur_cnt = 0

        if worker.should_stop == 1: return
        worker.dbcon.query(msg.g_init_sql_inittask_view)
        result = worker.dbcon.show()
        if result:
            for row in result:
                if worker.should_stop == 1: return
                worker.dbcon.query(msg.g_init_sql_inittask_view_inited % (row[0],))
                print >> sys.stderr,  row
                msgobj.append({'id':row[0], 'opt':msg.g_opt_add, 'view':row[1], 'mask':row[2]})
                count += 1
                if msg.g_init_sendstate == 1 and expect_cnt-count < 10:
                    msg.g_init_sendstate = 2
                    req_handler.set_chk_init_timer(worker)
                cur_cnt += 1
                if cur_cnt >= mgr_conf.g_row_perpack4init:
                    if worker.sendto_(msgobj, addr, msg.g_pack_head_init_view, mgr_conf.g_reply_port) != True:
                        return
                    cur_cnt = 0
                    del msgobj[:]

            if cur_cnt > 0:
                if worker.sendto_(msgobj, addr, msg.g_pack_head_init_view, mgr_conf.g_reply_port) != True:
                    return

        msgobj = {'complete':1}
        worker.sendto_(msgobj, addr, msg.g_pack_head_init_complete, mgr_conf.g_reply_port)
        msg.g_init_resp_expect = count

    @staticmethod
    def handle_proxy_init_reply(worker, answ, addr):
        str_class = answ['class'].partition('_')[0]
        state_set = 0
        if answ['type'] == msg.g_ptr_tbl:
            answ['data'] = partition_reserve_ip_from_ptr(answ['data'])
        if answ['result'] == 0:
            state_set = 1
        if answ['result'] == 1:
            state_set = 3
        if answ['result'] == 2:
            state_set = 2

        print >> sys.stderr,  ("tid:" + str(answ['id']) + ' ' + str_class + ' ' + str(answ['type']) + ' ' + str(answ['viewid']) +
                ' ' + str(answ['data']) + ' ' + str(state_set) + ' ' + str(answ['opt']))

        if worker.just4testcnt > 0:
            worker.dbcon.query(msg.g_init_sql_replytask % (state_set, answ['id']))

        if False and worker.just4testcnt > 0:
            if str_class == 'dns':
                mgr_err_describe.g_err_desc.del_record_timeout(answ['opt'], answ['viewid'], answ['data'], answ['type'])
            if str_class == 'view':
                mgr_err_describe.g_err_desc.del_view_timeout(answ['opt'], answ['viewid'], answ['data'])

        if msg.g_init_sendstate == 2 and msg.g_init_maxid-int(answ['id']) < 10:
            req_handler.set_chk_snd_timer(worker)
            msg.g_init_sendstate = 0

    @staticmethod
    def handle_inner_chk_init_ok(worker):
        worker.dbcon.query(msg.g_init_sql_chk_init_ok)
        result = worker.dbcon.show()
        if result:
            return result[0][0]

    @staticmethod
    def handle_inner_chk_snd(worker):
        msgobj = []
        worker.dbcon.query(msg.g_inner_sql_chksnd_dns)
        result = worker.dbcon.show()
        if result:
            for row in result:
                worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
                msgobj.append({'id':row[0],'opt':row[4],'domain':row[3],'view':row[2],'type':row[1],
                    'pkt_head':msg.g_pack_head_init_dns})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])
                if False:
                    mgr_err_describe.g_err_desc.add_record_timeout(row[4], row[2], row[3], row[1])
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], True)

        del msgobj[:]
        worker.dbcon.query(msg.g_inner_sql_chksnd_view)
        result = worker.dbcon.show()
        if result:
            for row in result:
                worker.dbcon.query(msg.g_init_sql_inittask_view_inited % (row[0],))
                msgobj.append({'id':row[0],'opt':row[3],'view':row[1],'mask':row[2],
                    'pkt_head':msg.g_pack_head_init_view})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])
                if False:
                    mgr_err_describe.g_err_desc.add_view_timeout(row[3], row[1], row[2])
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], True)

    @staticmethod
    def handle_inner_chk_task(http_th, worker, _type):
        payload = {'type': msg.g_class_inner_reqtype_map[_type],
                'opt': 'get',
                'ioopt': '\xe4\xb8\xad\xe6\x96\x87',
                'data': {'sid': mgr_conf.g_mgr_sid}}
        payload_encode = 'data=' + json.dumps(payload)
        print >> sys.stderr,  ('post encode data:\n' + repr(payload_encode))
        test_de = json.loads(payload_encode[5:])['ioopt']
        res, post_error = http_th.http_send_post(mgr_conf.g_url_inner_chk_task_ip, mgr_conf.g_url_inner_chk_task_url, payload_encode)
        if not res:
            raise Exception('request task post code:', post_error)
        print >> sys.stderr,  ('request task return:\n' + repr(res))
        decodejson = json.loads(res)
        print >> sys.stderr,  ('json ret:' + repr(decodejson['ret']))
        print >> sys.stderr,  ('json error:' + repr(decodejson['error']))
        print >> sys.stderr,  ('json result:\n' + repr(decodejson['result']))
        if decodejson['ret'] != 0:
            raise Exception('request task return error! ret:%d error:%s' % (decodejson['ret'], decodejson['error']))
        decodejson['class'] = msg.g_class_inner_map[_type]
        decodejson.pop('error')
        decodejson.pop('ret')
        worker.handler(decodejson)

    @staticmethod
    def handle_inner_chk_task_reply(worker, data):
        dic_result = data['result']
        if len(dic_result) < 1:
            print >> sys.stderr,  (data['class'] + ' no task')
            return
        replymsg = {'class': msg.g_class_inner_chk_task_done,
                'opt': 'set',
                'data': {'sid': mgr_conf.g_mgr_sid,
                    'tasks': {}}}
        ptr_tasks = replymsg['data']['tasks']
        msgobj = []
        bat_task = []
        for task_id in dic_result:
            task_data = dic_result.get(task_id)
            print >> sys.stderr,  (task_id + '\t:' + repr(task_data))
            try:
                dedata = json.loads(task_data['data'])
            except Exception as e:
                print >> sys.stderr,  ('load json data error!' + repr(e))
                continue

            task_type = task_data['type']
            replymsg['type'] = task_type
            found = False
            ali = None
            if task_type == 'record':
                for ali in http_tbl_alise:
                    if dedata.has_key(ali):
                        found = True
                        break
            else:
                ali = '__any__'
                found = True

            if not found:
                ptr_tasks[task_id] = {'ret': 1, 'result': 'task id ' + task_id + ' failed', 'error': 'table error'}
                continue
            try:
                db_ret, go_on, old_data = worker.m_handlers[task_type][ali].callme(worker, dedata, ali, task_data['opt'])
            except Exception as e:
                print >> sys.stderr,  ('!!!!--->' + repr(e))
                ptr_tasks[task_id] = {'ret': 1, 'result': 'task id ' + task_id + ' failed', 'error': 'set db failed!'}
                continue

            if not db_ret:
                ptr_tasks[task_id] = {'ret': 1, 'result': 'task id ' + task_id + ' failed', 'error': 'db error'}
                continue
            ptr_tasks[task_id] = {'ret': 0, 'result': 'task id ' + task_id + ' succ', 'error': ''}
            if go_on:
                try:
                    is_bat_task = worker.m_handlers[task_type][ali].notify(worker, msgobj, opt=task_data['opt'], data=dedata, odata=old_data)
                    if is_bat_task:
                        dedata['handle_class'] = worker.m_handlers[task_type][ali]
                        bat_task.append(dedata)
                except Exception as e:
                    print >> sys.stderr,  ('notify proxy error!!!!--->' + repr(e))
                    continue

        req_handler.notify_flush(worker, msgobj)
        worker.http_th.handler(replymsg)

        print >> sys.stderr, ('bat tasks:' + repr(bat_task))
        for dedata in bat_task:
            try:
                dedata['handle_class'].bat_notify(worker, dedata)
            except Exception as e:
                print >> sys.stderr, ('handle bat task:' + repr(dedata), '\n\terror!!!!--->' + repr(e))
                continue

    @staticmethod
    def handle_inner_chk_task_done(http_th, data_done):
        data_done.pop('class')
        payload_encode = 'data=' + json.dumps(data_done)
        print >> sys.stderr,  ('post data:\n' + repr(payload_encode))
        res, post_error = http_th.http_send_post(mgr_conf.g_url_inner_chk_task_ip, mgr_conf.g_url_inner_chk_task_url, payload_encode)
        if not res:
            raise Exception('request task post code:', post_error)
        print >> sys.stderr,  ('request task return:\n' + repr(res))
        decodejson = json.loads(res)
        print >> sys.stderr,  ('json ret:' + repr(decodejson['ret']))
        print >> sys.stderr,  ('json error:' + repr(decodejson['error']))
        print >> sys.stderr,  ('json result:\n' + repr(decodejson['result']))

    @staticmethod
    def handle_proxy_heartbeat(worker, data):
        print >> sys.stderr, ('g_err_desc type is ' + str(type(mgr_err_describe.g_err_desc)))
        objs = mgr_err_describe.g_err_desc.gen_msg()
        data['message'] = objs
        data['status'] = len(objs[0])>0 and -1 or 0
        print >> sys.stderr, ('heatbeat payload:\n' + repr(objs))
        worker.reply_echo(data, data['inner_addr'][0], data['inner_addr'][1])

class req_handler_record_a(object):
    def callme(self, worker, data, ali_tbl, opt):
        if opt == 'add':
            return self.add(worker, data, ali_tbl)
        elif opt == 'set':
            return self.set(worker, data, ali_tbl)
        elif opt == 'del':
            return self.delete(worker, data, ali_tbl)
        else:
            print >> sys.stderr,  ('opt:' + str(opt) + ' not implement')
            raise Exception('opt not implement')

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:a_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (add_ret, True, result)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:a_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (update_ret, True, result)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:a_record rid:' + str(data['rid']) + ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (True, True, result)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        if len(worker.proxy_addr.keys()) < 1:
            return False
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(odata))
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            msgobj.append({'opt':row[4], 'domain':row[3], 'view':row[2], 'type':row[1], 'id':row[0],
                'pkt_head':msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        return False

class req_handler_record_ptr(object):
    def callme(self, worker, data, ali_tbl, opt):
        if opt == 'add':
            return self.add(worker, data, ali_tbl)
        elif opt == 'set':
            return self.set(worker, data, ali_tbl)
        elif opt == 'del':
            return self.delete(worker, data, ali_tbl)
        else:
            print >> sys.stderr,  ('opt:' + str(opt) + ' not implement')
            raise Exception('opt not implement')

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:ptr_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_ptr_record,
                (data['name'], data[ali_tbl], int(data['viewid']), int(data['ttl']), 0, n_enable, int(data['rid'])) )
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return add_ret, True, result

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:ptr_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_ptr_record,
                (data['name'], data[ali_tbl], int(data['viewid']), int(data['ttl']), 0, n_enable, int(data['rid'])) )
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return update_ret, True, result

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:ptr_record rid:' + str(data['rid']) + ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (True, True, result)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        if len(worker.proxy_addr.keys()) < 1:
            return False
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(odata))
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            msgobj.append({'opt':row[4], 'domain':row[3], 'view':row[2], 'type':row[1], 'id':row[0],
                'pkt_head':msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        return False

class req_handler_record_aaaa(object):
    def callme(self, worker, data, ali_tbl, opt):
        if opt == 'add':
            return self.add(worker, data, ali_tbl)
        elif opt == 'set':
            return self.set(worker, data, ali_tbl)
        elif opt == 'del':
            return self.delete(worker, data, ali_tbl)
        else:
            print >> sys.stderr,  ('opt:' + str(opt) + ' not implement')
            raise Exception('opt not implement')

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:aaaa_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        print >> sys.stderr,  ('decode aaaa:' + str(data[ali_tbl]))
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (add_ret, True, result)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:aaaa_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        print >> sys.stderr,  ('decode aaaa:' + str(data[ali_tbl]))
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (update_ret, True, result)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:aaaa_record rid:' + str(data['rid']) + ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (True, True, result)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        if len(worker.proxy_addr.keys()) < 1:
            return False
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(odata))
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            msgobj.append({'opt':row[4], 'domain':row[3], 'view':row[2], 'type':row[1], 'id':row[0],
                'pkt_head':msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        return False

class req_handler_record_cname(object):
    def callme(self, worker, data, ali_tbl, opt):
        if opt == 'add':
            return self.add(worker, data, ali_tbl)
        elif opt == 'set':
            return self.set(worker, data, ali_tbl)
        elif opt == 'del':
            return self.delete(worker, data, ali_tbl)
        else:
            print >> sys.stderr,  ('opt:' + str(opt) + ' not implement')
            raise Exception('opt not implement')

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:cname_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (add_ret, True, result)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:cname_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (update_ret, True, result)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:cname_record rid:' + str(data['rid']) + ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (True, True, result)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        if len(worker.proxy_addr.keys()) < 1:
            return
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(odata))
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            msgobj.append({'opt':row[4], 'domain':row[3], 'view':row[2], 'type':row[1], 'id':row[0],
                'pkt_head':msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

class req_handler_record_ns(object):
    def callme(self, worker, data, ali_tbl, opt):
        if opt == 'add':
            return self.add(worker, data, ali_tbl)
        elif opt == 'set':
            return self.set(worker, data, ali_tbl)
        elif opt == 'del':
            return self.delete(worker, data, ali_tbl)
        else:
            print >> sys.stderr,  ('opt:' + str(opt) + ' not implement')
            raise Exception('opt not implement')

    def add_subrecord_inline(self, worker, str_main, n_vid, add_data):
        add_ret = worker.dbcon.call_proc(msg.g_proc_get_subrecord_inline, (str_main, n_vid))
        ars = worker.dbcon.show()
        hasnext = True
        while hasnext == True:
            if ars and len(ars) > 0:
                for i in range(len(ars)):
                    add_data.append(ars[i])
            hasnext = worker.dbcon.nextset()
            if None == hasnext:
                hasnext = False
            else:
                ars = worker.dbcon.show()
        worker.dbcon.fetch_proc_reset()
        return add_ret

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:ns_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (add_ret, True, result)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:ns_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (update_ret, True, result)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:ns_record rid:' + str(data['rid']) + ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (True, True, result)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        if len(worker.proxy_addr.keys()) < 1:
            return False
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(odata))
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            msgobj.append({'opt':row[4], 'domain':row[3], 'view':row[2], 'type':row[1], 'id':row[0],
                'pkt_head':msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        return True

    def bat_notify(self, worker, data):
        if len(worker.proxy_addr.keys()) < 1 or not data.has_key('main'):
            return
        print >> sys.stderr, ('bat_data:' + repr(data))
        sub_data = []
        sub_ret = self.add_subrecord_inline(worker, data['main'], int(data['viewid']), sub_data)
        print >> sys.stderr, ('updating subrecord:' + repr(sub_data))
        for record in sub_data:
            worker.dbcon.call_proc(msg.g_proc_add_task, ('dns', record[1], int(data['viewid']), record[0], 0, msg.g_opt_add))

        cur_cnt = 0
        msgobj = []
        worker.dbcon.query(msg.g_init_sql_inittask_dns)
        result = worker.dbcon.show()
        for record in result:
            msgobj.append({'id':record[0],'opt':msg.g_opt_add, 'domain':record[3], 'view':row[2], 'type':record[1]})
            cur_cnt += 1
            if cur_cnt >= mgr_conf.g_row_perpack4init:
                if worker.sendto_(msgobj, worker.proxy_addr.keys()[0], msg.g_pack_head_init_dns, mgr_conf.g_reply_port) != True:
                    return
                cur_cnt = 0
                del msgobj[:]
                time.sleep(1)
        if cur_cnt > 0:
            if worker.sendto_(msgobj, worker.proxy_addr.keys()[0], msg.g_pack_head_init_dns, mgr_conf.g_reply_port) != True:
                return
            time.sleep(1)

class req_handler_record_txt(object):
    def callme(self, worker, data, ali_tbl, opt):
        if opt == 'add':
            return self.add(worker, data, ali_tbl)
        elif opt == 'set':
            return self.set(worker, data, ali_tbl)
        elif opt == 'del':
            return self.delete(worker, data, ali_tbl)
        else:
            print >> sys.stderr,  ('opt:' + str(opt) + ' not implement')
            raise Exception('opt not implement')

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:txt_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        print >> sys.stderr,  ('decode TXT:' + str(data[ali_tbl]))
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (add_ret, True, result)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:txt_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        print >> sys.stderr,  ('decode TXT:' + str(data[ali_tbl]))
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (update_ret, True, result)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:txt_record rid:' + str(data['rid']) + ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (True, True, result)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        if len(worker.proxy_addr.keys()) < 1:
            return False
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(odata))
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            msgobj.append({'opt':row[4], 'domain':row[3], 'view':row[2], 'type':row[1], 'id':row[0],
                'pkt_head':msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        return False


class req_handler_record_mx(object):
    def callme(self, worker, data, ali_tbl, opt):
        if opt == 'add':
            return self.add(worker, data, ali_tbl)
        elif opt == 'set':
            return self.set(worker, data, ali_tbl)
        elif opt == 'del':
            return self.delete(worker, data, ali_tbl)
        else:
            print >> sys.stderr,  ('opt:' + str(opt) + ' not implement')
            raise Exception('opt not implement')

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:mx_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_mx_record, (data['name'].lstrip('@.'), data['main'],
            data['viewid'], data['ttl'], data['level'], data[ali_tbl], 0, n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (add_ret, True, result)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:mx_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_mx_record, (data['name'].lstrip('@.'), data['main'],
            data['viewid'], data['ttl'], data['level'], data[ali_tbl], 0, n_enable, data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (update_ret, True, result)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:mx_record rid:' + str(data['rid']) + ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (True, True, result)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        if len(worker.proxy_addr.keys()) < 1:
            return False
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(odata))
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            msgobj.append({'opt':row[4], 'domain':row[3], 'view':row[2], 'type':row[1], 'id':row[0],
                'pkt_head':msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        return False

class req_handler_record_domain_ns(object):
    def callme(self, worker, data, ali_tbl, opt):
        if opt == 'add':
            return self.add(worker, data, ali_tbl)
        elif opt == 'set':
            return self.set(worker, data, ali_tbl)
        elif opt == 'del':
            return self.delete(worker, data, ali_tbl)
        else:
            print >> sys.stderr,  ('opt:' + str(opt) + ' not implement')
            raise Exception('opt not implement')

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding domain_ns:' + str(data['main']) + '-->' + str(data['domain_ns']) + ' into database')
        add_ret = worker.dbcon.query(msg.g_sql_add_a_domain_ns % (data['main'], int(data['ttl']), data['domain_ns'],
            data['rid'], data['main'], int(data['ttl']), data['domain_ns']))
        print >> sys.stderr,  ('add return ' + str(add_ret))
        return (add_ret, False, None)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating domain_ns:' + str(data['main']) + '-->' + str(data['domain_ns']) + ' into database')
        add_ret = worker.dbcon.query(msg.g_sql_add_a_domain_ns % (data['main'], int(data['ttl']), data['domain_ns'],
            data['rid'], data['main'], int(data['ttl']), data['domain_ns']))
        return (add_ret, False, None)

class req_handler_domain(object):
    def callme(self, worker, data, ali_tbl, opt):
        if opt == 'add':
            return self.add(worker, data, ali_tbl)
        elif opt == 'set':
            return self.set(worker, data, ali_tbl)
        elif opt == 'del':
            return self.delete(worker, data, ali_tbl)
        else:
            print >> sys.stderr,  ('opt:' + str(opt) + ' not implement')
            raise Exception('opt not implement')

    def set(self, worker, data, ali_tbl):
        n_enable = 1 if int(data['enable']) == 0 else 0
        print >> sys.stderr,  ('update domain:' + str(data['name']) + '[' + str(n_enable) + '] from database')
        worker.dbcon.call_proc(msg.g_proc_set_a_domain, (data['name'], n_enable))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (True, True, result)

    def delete(self, worker, data, ali_tbl):
        if False:
            data['enable'] = 0
            return self.set(worker, data, ali_tbl)
        print >> sys.stderr,  ('deleting domain:' + str(data['name']) + ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_domain, (data['name'],))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return (True, True, result)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        if len(worker.proxy_addr.keys()) < 1:
            return False
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(odata))
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            msgobj.append({'opt':row[4], 'domain':row[3], 'view':row[2], 'type':row[1], 'id':row[0],
                'pkt_head':msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        return False

http_tbl_alise = ('A', 'AAAA', 'PTR', 'CNAME', 'NS', 'TXT', 'MX', 'domain_ns')
http_tbl_realname = {'A': 'a_record',
		'PTR' : 'ptr_record',
        'AAAA': 'aaaa_record',
        'CNAME': 'cname_record',
        'NS': 'ns_record',
        'TXT': 'txt_record',
        'MX': 'mx_record',
        'domain_ns': 'domain_ns'}
http_opt_str2int = {'add': msg.g_opt_add, 'del': msg.g_opt_del}
http_type_to_proxy_header = {'record': msg.g_pack_head_init_dns, 'domain': msg.g_pack_head_init_view}

