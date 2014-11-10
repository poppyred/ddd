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
    def handle_proxy_init(worker, addr):
        msgobj = []
        count = 0
        worker.dbcon.query(msg.g_init_sql_view)
        result = worker.dbcon.show()
        if result:
            for row in result:
                print >> sys.stderr,  row
                worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('view', 0, row[0], row[1], 0, msg.g_opt_add))
                msgobj.append({'opt': msg.g_opt_add, 'view': row[0], 'mask': row[1], 'pkt_head': msg.g_pack_head_init_view})
                count += 1
                req_handler.notify_proxy(worker, msgobj, addr)

            req_handler.notify_proxy(worker, msgobj, addr, True)

        del msgobj[:]
        worker.dbcon.query(msg.g_sql_clean_snd_req)
        for atbl in msg.g_list_tbl:
            worker.dbcon.query(msg.g_init_sql_dns % atbl)
            result = worker.dbcon.show()
            if not result:
                continue
            for row in result:
                print >> sys.stderr,  'dns query %s res: %s' % (atbl, row)
                worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('dns', msg.g_dict_type[atbl], row[1], row[0],
                    0, msg.g_opt_add))
                msgobj.append({'opt': msg.g_opt_add, 'domain': row[0], 'view': row[1], 'type': msg.g_dict_type[atbl],
                    'pkt_head': msg.g_pack_head_init_dns})
                count += 1
                req_handler.notify_proxy(worker, msgobj, addr)
                if atbl == 'cname_record':
                    print >> sys.stderr,  'send 1 more A for CNAME : %s' % (row[0],)
                    worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('dns', msg.g_dict_type['a_record'], row[1], row[0],
                        0, msg.g_opt_add))
                    msgobj.append({'opt': msg.g_opt_add, 'domain': row[0], 'view': row[1], 'type': msg.g_dict_type['a_record'],
                        'pkt_head': msg.g_pack_head_init_dns})
                    count += 1
                    req_handler.notify_proxy(worker, msgobj, addr)

            req_handler.notify_proxy(worker, msgobj, addr, True)

        msg.g_init_resp_expect = count

    @staticmethod
    def handle_proxy_init_new(worker, addr):
        worker.dbcon.query(msg.g_sql_clean_snd_req)

        msgobj = []
        count = 0
        cur_cnt = 0

        for atbl in msg.g_list_tbl:
            worker.dbcon.query(msg.g_init_sql_dns % atbl)
            result = worker.dbcon.show()
            if not result:
                continue
            for row in result:
                print >> sys.stderr,  'dns query %s res: %s' % (atbl, row)
                if False:
                    worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('dns', msg.g_dict_type[atbl], row[1], row[0],
                        0, msg.g_opt_add))
                msgobj.append({'opt': msg.g_opt_add, 'domain': row[0], 'view': row[1], 'type': msg.g_dict_type[atbl]})
                count += 1
                cur_cnt += 1
                if atbl == 'cname_record':
                    print >> sys.stderr,  'send 1 more A for CNAME : %s' % (row[0],)
                    if False:
                        worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('dns', msg.g_dict_type['a_record'], row[1], row[0],
                            0, msg.g_opt_add))
                    msgobj.append({'opt': msg.g_opt_add, 'domain': row[0], 'view': row[1], 'type': msg.g_dict_type['a_record']})
                    count += 1
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

        worker.dbcon.query(msg.g_init_sql_view)
        result = worker.dbcon.show()
        if result:
            for row in result:
                print >> sys.stderr,  row
                if False:
                    worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('view', 0, row[0], row[1], 0, msg.g_opt_add))
                msgobj.append({'opt': msg.g_opt_add, 'view': row[0], 'mask': row[1]})
                count += 1
                cur_cnt += 1
                if cur_cnt >= mgr_conf.g_row_perpack4init:
                    if worker.sendto_(msgobj, addr, msg.g_pack_head_init_view, mgr_conf.g_reply_port) != True:
                        return
                    cur_cnt = 0
                    del msgobj[:]

            if cur_cnt > 0:
                if worker.sendto_(msgobj, addr, msg.g_pack_head_init_view, mgr_conf.g_reply_port) != True:
                    return

        msg.g_init_resp_expect = count
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_domain, mgr_conf.g_inner_chk_task_domain_time)
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_record, mgr_conf.g_inner_chk_task_record_time)
        print >> sys.stderr,  'set timers'

    @staticmethod
    def handle_proxy_init_reply(worker, answ, addr):
        str_class = answ['class'].partition('_')[0]
        state_set = 0
        if answ['result'] == 0:
            state_set = 1
        if answ['result'] == 1:
            state_set = 3
        if answ['result'] == 2:
            state_set = 2

        print >> sys.stderr,  (str_class + ' ' + str(answ['type']) + ' ' + str(answ['viewid']) + ' ' + str(answ['data']) + ' ' + str(state_set) + ' ' + str(answ['opt']))

        if worker.just4testcnt > 0:
            worker.dbcon.call_proc(msg.g_proc_update_snd_req, (str_class, answ['type'], answ['viewid'],
                answ['data'], state_set, answ['opt']))

        if worker.just4testcnt > 0:
            if str_class == 'dns':
                mgr_err_describe.g_err_desc.del_record_timeout(answ['opt'], answ['viewid'], answ['data'], answ['type'])
            if str_class == 'view':
                mgr_err_describe.g_err_desc.del_view_timeout(answ['opt'], answ['viewid'], answ['data'])

    @staticmethod
    def handle_inner_chk_init_ok(worker):
        worker.dbcon.query(msg.g_init_sql_chk_init_ok)
        result = worker.dbcon.show()
        if result:
            return result[0][0]

    @staticmethod
    def handle_inner_chk_snd(worker):
        msgobj = []
        worker.dbcon.query(msg.g_inner_sql_chksnd_view)
        result = worker.dbcon.show()
        if result:
            for row in result:
                msgobj.append({'opt': row[2], 'view': row[0], 'mask': row[1],
                    'pkt_head': msg.g_pack_head_init_view})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])
                mgr_err_describe.g_err_desc.add_view_timeout(row[2], row[0], row[1])

            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], True)
        del msgobj[:]
        worker.dbcon.query(msg.g_inner_sql_chksnd_dns)
        result = worker.dbcon.show()
        if result:
            for row in result:
                msgobj.append({'opt': row[3], 'domain': row[2], 'view': row[1], 'type': row[0],
                    'pkt_head': msg.g_pack_head_init_dns})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])
                mgr_err_describe.g_err_desc.add_record_timeout(row[3], row[1], row[2], row[0])

            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], True)

    @staticmethod
    def travel_dictionary(decodejson):
        for k in decodejson:
            v = decodejson.get(k)
            if type(v) is types.ListType:
                print >> sys.stderr, str(k) + '---'
                for kk, vv in enumerate(v):
                    print >> sys.stderr, str(kk) + str(vv)
                    print >> sys.stderr, '---'

            else:
                print >> sys.stderr, str(k) + '\t:' + str(decodejson.get(k))

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
                    worker.m_handlers[task_type][ali].notify(worker, msgobj, opt=task_data['opt'], data=dedata, odata=old_data)
                except Exception as e:
                    print >> sys.stderr,  ('notify proxy error!!!!--->' + repr(e))
                    continue

        req_handler.notify_flush(worker, msgobj)
        worker.http_th.handler(replymsg)

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
    def handle_inner_chk_task_db_heartbeat(worker):
        worker.dbcon.query(msg.g_inner_sql_db_heartbeat)
        result = worker.dbcon.show()
        print >> sys.stderr,  repr(result)
        if not result:
            print >> sys.stderr,  'reconnecting to mysql!!!!!'
            worker.dbcon.query(msg.g_inner_sql_db_heartbeat)

    @staticmethod
    def handle_proxy_heartbeat(worker, data):
        print >> sys.stderr, ('g_err_desc type is ' + str(type(mgr_err_describe.g_err_desc)))
        objs = mgr_err_describe.g_err_desc.gen_msg()
        data['message'] = objs
        data['status'] = len(objs[0])>0 and -1 or 0
        print >> sys.stderr, ('heatbeat payload:\n' + repr(objs))
        worker.reply_echo(data, data['inner_addr'][0], data['inner_addr'][1])

class req_hdl_abstract(object):
    def __init__(self):
        pass

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  'not implement'
        raise Exception('opt not implement')

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  'not implement'
        raise Exception('opt not implement')

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  'not implement'
        raise Exception('opt not implement')

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        print >> sys.stderr,  'not implement'
        raise Exception('opt not implement')

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

class req_handler_impl(req_hdl_abstract):
    def __init__(self):
        req_hdl_abstract.__init__(self)

    def add(self, worker, data, ali_tbl):
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])

            worker.dbcon.nextset()
            ars = worker.dbcon.show()

        worker.dbcon.fetch_proc_reset()
        print >> sys.stderr,  ('select old:' + str(result))
        return (add_ret, True, result)

    def set(self, worker, data, ali_tbl):
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record, (http_tbl_realname[ali_tbl],
            data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data[ali_tbl], 0,
            n_enable, data['rid']))
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])

            worker.dbcon.nextset()
            ars = worker.dbcon.show()

        worker.dbcon.fetch_proc_reset()
        print >> sys.stderr,  ('select old:' + str(result))
        return (update_ret, True, result)

    def delete(self, worker, data, ali_tbl):
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        result = worker.dbcon.fetch_proc_reset()
        print >> sys.stderr,  ('select old:' + str(result))
        if result[0][0] == None or result[0][1] == None:
            result = None
        return (True, True, result)

    def donotify(self, worker, msgobj, opt, data, odata, real_tbl):
        if len(worker.proxy_addr.keys()) < 1:
            return
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(data))

        if opt == 'add':
            if odata and len(odata) > 0 and len(odata[0]) >= 4:
                odata0 = odata[0]
                ropt = 'del'
                if odata0[3] > 0:
                    ropt = 'add'
                msgobj.append({'opt': http_opt_str2int[ropt], 'domain': odata0[0], 'view': odata0[1],
                    'type': msg.g_dict_type[odata0[2]], 'pkt_head': msg.g_pack_head_init_dns})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
            msgobj.append({'opt': http_opt_str2int['add'], 'domain': data['name'].lstrip('@.'),
                'view': int(data['viewid']), 'type': msg.g_dict_type[real_tbl],
                'pkt_head': msg.g_pack_head_init_dns})

        if opt == 'set':
            if odata and len(odata) > 0 and len(odata[0]) >= 4:
                odata0 = odata[0]
                ropt = 'del'
                if odata0[3] > 0:
                    ropt = 'add'
                msgobj.append({'opt': http_opt_str2int[ropt], 'domain': odata0[0], 'view': odata0[1],
                    'type': msg.g_dict_type[odata0[2]], 'pkt_head': msg.g_pack_head_init_dns})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
            ropt = 'add'
            if data.has_key('enable') and int(data['enable']) == 0:
                ropt = 'del'
            msgobj.append({'opt': http_opt_str2int[ropt], 'domain': data['name'].lstrip('@.'),
                'view': int(data['viewid']), 'type': msg.g_dict_type[real_tbl],
                'pkt_head': msg.g_pack_head_init_dns})

        if opt == 'del':
            if odata and len(odata) > 0 and len(odata[0]) >= 3:
                odata0 = odata[0]
                ropt = opt
                if odata0[2] > 0:
                    ropt = 'add'
                msgobj.append({'opt': http_opt_str2int[ropt], 'domain': odata0[0], 'view': odata0[1],
                    'type': msg.g_dict_type[real_tbl], 'pkt_head': msg.g_pack_head_init_dns})

        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)


class req_handler_record_a(req_handler_impl):

    def __init__(self):
        req_handler_impl.__init__(self)

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:a_record into database')
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:a_record into database')
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:a_record rid:' + str(data['rid']) + ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        self.donotify(worker, msgobj, opt, data, odata, 'a_record')


class req_handler_record_aaaa(req_handler_impl):

    def __init__(self):
        req_handler_impl.__init__(self)

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:aaaa_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        print >> sys.stderr,  ('decode aaaa:' + str(data[ali_tbl]))
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:aaaa_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        print >> sys.stderr,  ('decode aaaa:' + str(data[ali_tbl]))
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:aaaa_record rid:' + str(data['rid']) + ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        self.donotify(worker, msgobj, opt, data, odata, 'aaaa_record')


class req_handler_record_cname(req_handler_impl):

    def __init__(self):
        req_handler_impl.__init__(self)

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:cname_record into database')
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:cname_record into database')
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:cname_record rid:' + str(data['rid']) + ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        self.donotify(worker, msgobj, opt, data, odata, 'cname_record')

    def send1more(self, worker, msgobj, tblname, domain, view, ropt):
        if tblname == 'cname_record':
            print >> sys.stderr,  'send 1 more A for CNAME : %s, opt : %s' % (domain, ropt)
            msgobj.append({'opt': http_opt_str2int[ropt], 'domain': domain, 'view': view,
                'type': msg.g_dict_type['a_record'], 'pkt_head': msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

    def donotify(self, worker, msgobj, opt, data, odata, real_tbl):
        if len(worker.proxy_addr.keys()) < 1:
            return
        print >> sys.stderr,  ('opt:' + str(opt) + ' data:' + str(data))
        if opt == 'add':
            if odata and len(odata) > 0 and len(odata[0]) >= 4:
                odata0 = odata[0]
                ropt = 'del'
                if odata0[3] > 0:
                    ropt = 'add'
                msgobj.append({'opt': http_opt_str2int[ropt], 'domain': odata0[0], 'view': odata0[1],
                    'type': msg.g_dict_type[odata0[2]], 'pkt_head': msg.g_pack_head_init_dns})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
                self.send1more(worker, msgobj, odata0[2], odata0[0], odata0[1], ropt)
            msgobj.append({'opt': http_opt_str2int[opt], 'domain': data['name'].lstrip('@.'),
                'view': int(data['viewid']), 'type': msg.g_dict_type[real_tbl],
                'pkt_head': msg.g_pack_head_init_dns})
            self.send1more(worker, msgobj, real_tbl, data['name'], int(data['viewid']), opt)

        if opt == 'set':
            if odata and len(odata) > 0 and len(odata[0]) >= 4:
                odata0 = odata[0]
                ropt = 'del'
                if odata0[3] > 0:
                    ropt = 'add'
                msgobj.append({'opt': http_opt_str2int[ropt], 'domain': odata0[0], 'view': odata0[1],
                    'type': msg.g_dict_type[odata0[2]], 'pkt_head': msg.g_pack_head_init_dns})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
                self.send1more(worker, msgobj, odata0[2], odata0[0], odata0[1], ropt)
            ropt = 'add'
            if data.has_key('enable') and int(data['enable']) == 0:
                ropt = 'del'
            msgobj.append({'opt': http_opt_str2int[ropt], 'domain': data['name'].lstrip('@.'),
                'view': int(data['viewid']), 'type': msg.g_dict_type[real_tbl], 'pkt_head': msg.g_pack_head_init_dns})
            self.send1more(worker, msgobj, real_tbl, data['name'], int(data['viewid']), ropt)

        if opt == 'del':
            if odata and len(odata) > 0 and len(odata[0]) >= 3:
                odata0 = odata[0]
                ropt = opt
                if odata0[2] > 0:
                    ropt = 'add'
                msgobj.append({'opt': http_opt_str2int[ropt], 'domain': odata0[0], 'view': odata0[1],
                    'type': msg.g_dict_type[real_tbl], 'pkt_head': msg.g_pack_head_init_dns})
                self.send1more(worker, msgobj, real_tbl, odata0[0], odata0[1], ropt)

        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)


class req_handler_record_ns(req_handler_impl):

    def __init__(self):
        req_handler_impl.__init__(self)

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:ns_record into database')
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:ns_record into database')
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:ns_record rid:' + str(data['rid']) + ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        self.donotify(worker, msgobj, opt, data, odata, 'ns_record')


class req_handler_record_txt(req_handler_impl):

    def __init__(self):
        req_handler_impl.__init__(self)

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:txt_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        print >> sys.stderr,  ('decode TXT:' + str(data[ali_tbl]))
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:txt_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        print >> sys.stderr,  ('decode TXT:' + str(data[ali_tbl]))
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:txt_record rid:' + str(data['rid']) + ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        self.donotify(worker, msgobj, opt, data, odata, 'txt_record')


class req_handler_record_mx(req_handler_impl):

    def __init__(self):
        req_handler_impl.__init__(self)

    def add(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('adding name:' + str(data['name']) + ' table:mx_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_mx_record, (data['name'].lstrip('@.'), data['main'],
            data['viewid'], data['ttl'], data['level'], data[ali_tbl], 0, n_enable, data['rid']))
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])

            worker.dbcon.nextset()
            ars = worker.dbcon.show()

        worker.dbcon.fetch_proc_reset()
        print >> sys.stderr,  ('select old:' + str(result))
        return (add_ret, True, result)

    def set(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('updating name:' + str(data['name']) + ' table:mx_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_mx_record, (data['name'].lstrip('@.'), data['main'],
            data['viewid'], data['ttl'], data['level'], data[ali_tbl], 0, n_enable, data['rid']))
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])

            worker.dbcon.nextset()
            ars = worker.dbcon.show()

        worker.dbcon.fetch_proc_reset()
        print >> sys.stderr,  ('select old:' + str(result))
        return (update_ret, True, result)

    def delete(self, worker, data, ali_tbl):
        print >> sys.stderr,  ('deleting name:mx_record rid:' + str(data['rid']) + ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        self.donotify(worker, msgobj, opt, data, odata, 'mx_record')


class req_handler_record_domain_ns(req_hdl_abstract):

    def __init__(self):
        req_hdl_abstract.__init__(self)

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


class req_handler_domain(req_handler_impl):

    def __init__(self):
        req_handler_impl.__init__(self)

    def add(self, worker, data, ali_tbl):
        return req_hdl_abstract.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        n_enable = 1 if int(data['enable']) == 0 else 0
        print >> sys.stderr,  ('update domain:' + str(data['name']) + '[' + str(n_enable) + '] from database')
        worker.dbcon.call_proc(msg.g_proc_set_a_domain, (data['name'], n_enable))
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])

            worker.dbcon.nextset()
            ars = worker.dbcon.show()

        worker.dbcon.fetch_proc_reset()
        print >> sys.stderr,  ('select old:' + str(result))
        return (True, True, result)

    def delete(self, worker, data, ali_tbl):
        if False:
            data['enable'] = 0
            return self.set(worker, data, ali_tbl)
        print >> sys.stderr,  ('deleting domain:' + str(data['name']) + ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_domain, (data['name'],))
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])

            worker.dbcon.nextset()
            ars = worker.dbcon.show()

        worker.dbcon.fetch_proc_reset()
        print >> sys.stderr,  ('select old:' + str(result))
        return (True, True, result)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None):
        self.donotify(worker, msgobj, opt, data, odata)

    def donotify(self, worker, msgobj, opt = None, data = None, odata = None, real_tbl = None):
        print >> sys.stderr,  ('enter opt:' + str(opt) + ' data:' + str(data) + ' odata:' + str(odata))
        if len(worker.proxy_addr.keys()) < 1:
            return

        if opt == 'del':
            for od in odata:
                if len(od) >= 3 and od[0] and od[1] != None and od[2]:
                    print >> sys.stderr,  ('notify od for del:' + str(od))
                    msgobj.append({'opt': http_opt_str2int[opt],
                        'domain': od[0],
                        'view': od[1],
                        'type': msg.g_dict_type[od[2]],
                        'pkt_head': msg.g_pack_head_init_dns})
                    req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])

        if opt == 'set':
            ropt = 'add'
            if data.has_key('enable') and int(data['enable']) == 0:
                ropt = 'del'
            for od in odata:
                if len(od) >= 3 and od[0] and od[1] != None and od[2]:
                    print >> sys.stderr,  ('notify od for set:' + str(od))
                    msgobj.append({'opt': http_opt_str2int[ropt],
                        'domain': od[0],
                        'view': od[1],
                        'type': msg.g_dict_type[od[2]],
                        'pkt_head': msg.g_pack_head_init_dns})
                    req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])

class req_handler_view_mask(req_handler_impl):
    def __init__(self):
        req_handler_impl.__init__(self)

    def add(self, worker, data, ali_tbl):
        return req_hdl_abstract.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        return req_hdl_abstract.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        return req_hdl_abstract.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt = None, data = None, odata = None, flush = False, direct = False):
        return req_hdl_abstract.notify(self, worker, msgobj, opt, data, odata, '', flush, direct)


http_tbl_alise = ('A', 'AAAA', 'CNAME', 'NS', 'TXT', 'MX', 'domain_ns')
http_tbl_realname = {'A': 'a_record',
        'AAAA': 'aaaa_record',
        'CNAME': 'cname_record',
        'NS': 'ns_record',
        'TXT': 'txt_record',
        'MX': 'mx_record',
        'domain_ns': 'domain_ns'}
http_opt_str2int = {'add': msg.g_opt_add, 'del': msg.g_opt_del}
http_type_to_proxy_header = {'record': msg.g_pack_head_init_dns, 'domain': msg.g_pack_head_init_view}

