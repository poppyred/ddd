#!/usr/bin/python
#coding: utf-8

import MySQL
import msg
import sys
import json
import types
import mgr_conf
from mgr_misc import _lineno
import traceback
import urllib

__all__ = ['switch', 'req_handler', 'g_req_loger', 'req_handler_record_a', 'req_handler_record_aaaa', 'req_handler_record_cname',
        'req_handler_record_ns', 'req_handler_record_txt', 'req_handler_record_mx', 'req_handler_record_domain_ns',
        'req_handler_domain', 'req_handler_view_mask']

g_req_loger = None

class switch(object):
    def __init__(self, value):
        self.value = value
        self.fall = False

    def __iter__(self):
        """Return the match method once, then stop"""
        yield self.match
        raise StopIteration

    def match(self, *args):
        """Indicate whether or not to enter a case suite"""
        if self.fall or not args:
            return True
        elif self.value in args: # changed for v1.5, see below
            self.fall = True
            return True
        else:
            return False

class req_handler(object):
    @staticmethod
    def notify_proxy(worker, msgobj, addr, flush=False):
        if not flush and len(msgobj) >= mgr_conf.g_row_perpack or flush and len(msgobj) > 0:
            pre_pkt_head = None
            pre_i = 0
            for i in range(len(msgobj)):
                if msgobj[i].has_key('pkt_head'):
                    cur_pkt_head = msgobj[i].pop('pkt_head')
                else:
                    g_req_loger.error(_lineno(), 'no pkt_head key-->', repr(msgobj[i]))
                    cur_pkt_head = None
                if cur_pkt_head and (pre_pkt_head and cur_pkt_head != pre_pkt_head):
                    g_req_loger.info(_lineno(), 'pkt piece head:', pre_pkt_head, ' -->', repr(msgobj[pre_i:i]))
                    worker.reply(msgobj[pre_i:i], pre_pkt_head, addr)
                    pre_i = i
                if cur_pkt_head:
                    pre_pkt_head = cur_pkt_head

            if pre_pkt_head == None:
                g_req_loger.error(_lineno(), 'no pkt_head key-->', repr(msgobj))
            else:
                g_req_loger.care(_lineno(), 'pkt piece head:', pre_pkt_head, ' -->', repr(msgobj[pre_i:]))
                worker.reply(msgobj[pre_i:], pre_pkt_head, addr)

            del msgobj[:]

    @staticmethod
    def notify_flush(worker, msgobj):
        if len(worker.proxy_addr.keys()) < 1:
            return
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], True)

    @staticmethod
    def handle_proxy_init(worker, addr):
        worker.dbcon.query(msg.g_sql_clean_snd_req)
        #query dns
        msgobj = []
        count = 0
        for atbl in msg.g_list_tbl:
            worker.dbcon.query(msg.g_init_sql_dns % (atbl))
            result = worker.dbcon.show()
            if not result:
                continue
            for row in result:
                g_req_loger.debug(_lineno(), "dns query %s res: %s" % (atbl, row))
                worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('dns', msg.g_dict_type[atbl], row[1], row[0],
                    0, msg.g_opt_add))
                msgobj.append({'opt':msg.g_opt_add, 'domain':row[0], 'view':row[1], 'type':msg.g_dict_type[atbl],
                    'pkt_head':msg.g_pack_head_init_dns})
                count += 1
                req_handler.notify_proxy(worker, msgobj, addr)
            req_handler.notify_proxy(worker, msgobj, addr, True)

        #query view
        del msgobj[:]
        worker.dbcon.query(msg.g_init_sql_view)
        result = worker.dbcon.show()
        if result:
            for row in result:
                g_req_loger.debug(_lineno(), row)
                worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('view', 0, row[0], row[1], 0, msg.g_opt_add))
                msgobj.append({'opt':msg.g_opt_add, 'view':row[0], 'mask':row[1], 'pkt_head':msg.g_pack_head_init_view})
                count += 1
                req_handler.notify_proxy(worker, msgobj, addr)
            req_handler.notify_proxy(worker, msgobj, addr, True)

        msg.g_init_resp_expect = count

    @staticmethod
    def handle_proxy_init_new(worker, addr):
        worker.dbcon.query(msg.g_sql_clean_snd_req)
        #query dns
        msgobj = []
        count = 0
        cur_cnt = 0
        for atbl in msg.g_list_tbl:
            worker.dbcon.query(msg.g_init_sql_dns % (atbl))
            result = worker.dbcon.show()
            if not result:
                continue
            for row in result:
                g_req_loger.debug(_lineno(), "dns query %s res: %s" % (atbl, row))
                #worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('dns', msg.g_dict_type[atbl], row[1], row[0],
                #    0, msg.g_opt_add))
                msgobj.append({'opt':msg.g_opt_add, 'domain':row[0], 'view':row[1], 'type':msg.g_dict_type[atbl]})
                count += 1
                cur_cnt += 1
                if cur_cnt >= mgr_conf.g_row_perpack4init:
                    if worker.sendto_(msgobj, addr, msg.g_pack_head_init_dns) != True:
                        return
                    cur_cnt = 0
                    del msgobj[:]
        if cur_cnt > 0:
            if worker.sendto_(msgobj, addr, msg.g_pack_head_init_dns) != True:
                return

        #query view
        del msgobj[:]
        worker.dbcon.query(msg.g_init_sql_view)
        result = worker.dbcon.show()
        cur_cnt = 0
        if result:
            for row in result:
                g_req_loger.debug(_lineno(), row)
                #worker.dbcon.call_proc(msg.g_proc_add_snd_req, ('view', 0, row[0], row[1], 0, msg.g_opt_add))
                msgobj.append({'opt':msg.g_opt_add, 'view':row[0], 'mask':row[1]})
                count += 1
                cur_cnt += 1
                if cur_cnt >= mgr_conf.g_row_perpack4init:
                    if worker.sendto_(msgobj, addr, msg.g_pack_head_init_view) != True:
                        return
                    cur_cnt = 0
                    del msgobj[:]
            if cur_cnt > 0:
                if worker.sendto_(msgobj, addr, msg.g_pack_head_init_view) != True:
                    return

        msg.g_init_resp_expect = count
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_domain, mgr_conf.g_inner_chk_task_domain_time)
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_record, mgr_conf.g_inner_chk_task_record_time)

    @staticmethod
    def handle_proxy_init_reply(worker, answ, addr):
        str_class = answ['class'].partition('_')[0]
        state_set = 0
        for case in switch(answ['result']):
            if case(0):
                state_set = 1
                break
            if case(1): #操作失败
                state_set = 3
                break
            if case(2): #不存在
                state_set = 2
                break
            if case():
                state_set = 0
        g_req_loger.debug(_lineno(), str_class, ' ', answ['type'], ' ', answ['viewid'], ' ', answ['data'], ' ', state_set, ' ', answ['opt'])

        worker.dbcon.call_proc(msg.g_proc_update_snd_req, (str_class, answ['type'], answ['viewid'],
            answ['data'], state_set, answ['opt']))
        #worker.dbcon.query(msg.g_proc_update_snd_req_ret)
        #update_ret = worker.dbcon.show()
        #g_req_loger.debug(_lineno(), str_class, ' update return:', repr(update_ret))
        #if not update_ret or update_ret[0][0] != 1 :
        #    g_req_loger.error(_lineno(), '%s update database fail!!!!' % (str_class))

    @staticmethod
    def handle_inner_chk_init_ok(worker):
        worker.dbcon.query(msg.g_init_sql_chk_init_ok)
        result = worker.dbcon.show()
        if result:
            return result[0][0]
        return None

    @staticmethod
    def handle_inner_chk_snd(worker):
        msgobj = []
        worker.dbcon.query(msg.g_inner_sql_chksnd_view)
        result = worker.dbcon.show()
        if result:
            for row in result:
                msgobj.append({'opt':row[2], 'view':row[0], 'mask':row[1], 'pkt_head':msg.g_pack_head_init_view})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], True)

        del msgobj[:]
        worker.dbcon.query(msg.g_inner_sql_chksnd_dns)
        result = worker.dbcon.show()
        if result:
            for row in result:
                msgobj.append({'opt':row[3], 'domain':row[2], 'view':row[1], 'type':row[0], 'pkt_head':msg.g_pack_head_init_dns})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], True)

    @staticmethod
    def travel_dictionary(decodejson):
        for k in decodejson:
            v = decodejson.get(k)
            if type(v) is types.ListType: #如果数据是list类型，继续遍历
                print >> sys.stderr, k, '---'
                for kk, vv in enumerate(v):
                    print >> sys.stderr, kk, vv
                    print >> sys.stderr, '---'
            else:
                print >> sys.stderr, k, '\t:', decodejson.get(k)

    @staticmethod
    def handle_inner_chk_task(http_th, worker, _type):
        payload = { "type":msg.g_class_inner_reqtype_map[_type],
                "opt":"get",
                "ioopt":"中文",
                "data":{"sid":mgr_conf.g_mgr_sid} }
        payload_encode= 'data='+json.dumps(payload)
        g_req_loger.debug(_lineno(), 'post encode data:\n', repr(payload_encode))
        g_req_loger.debug(_lineno(), 'test decode data:\n', repr(json.loads(payload_encode[5:])))
        g_req_loger.debug(_lineno(), 'test pre ioopt--> utf8:', repr(payload['ioopt']))
        test_de = json.loads(payload_encode[5:])['ioopt']
        g_req_loger.debug(_lineno(), 'test enc and dec ioopt:', test_de.encode("UTF-8"), ', utf8:', repr(test_de.encode("UTF-8")), ', unicode:', repr(test_de))

        res, post_error = http_th.http_send_post(mgr_conf.g_url_inner_chk_task_ip,
                mgr_conf.g_url_inner_chk_task_url, payload_encode)
        if not res:
            g_req_loger.warn(traceback.format_exc())
            raise Exception(_lineno(), 'request task post code:', post_error)
        g_req_loger.debug(_lineno(), 'request task return:\n', repr(res))
        decodejson = json.loads(res)
        g_req_loger.debug(_lineno(), 'json ret:', repr(decodejson['ret']))
        g_req_loger.debug(_lineno(), 'json error:', repr(decodejson['error']))
        g_req_loger.debug(_lineno(), 'json result:\n', repr(decodejson['result']))
        if decodejson['ret'] != 0:
            g_req_loger.warn(traceback.format_exc())
            raise Exception(_lineno(), 'request task return error! \
                    ret:%d error:%s'%(decodejson['ret'], decodejson['error']))
        decodejson['class'] = msg.g_class_inner_map[_type]
        decodejson.pop('error')
        decodejson.pop('ret')
        worker.put(decodejson)

    @staticmethod
    def handle_inner_chk_task_reply(worker, data):
        dic_result = data['result']
        if len(dic_result) < 1:
            g_req_loger.info(_lineno(), data['class'], ' no task')
            return

        replymsg = {'class': msg.g_class_inner_chk_task_done,
                'opt' : 'set',
                'data' : {
                    'sid':mgr_conf.g_mgr_sid,
                    'tasks':{} } }
        ptr_tasks = replymsg['data']['tasks']
        msgobj = []
        for task_id in dic_result:
            task_data = dic_result.get(task_id)
            g_req_loger.info(_lineno(), task_id, '\t:', task_data)

            try:
                dedata = json.loads(task_data['data'])
            except Exception,e:
                g_req_loger.error(_lineno(), 'load json data error!', repr(e))
                g_req_loger.error(traceback.format_exc())
                continue

            task_type = task_data['type']
            replymsg['type'] = task_type

            found = False
            ali = None
            for case in switch(task_type):
                if case('record'):
                    for ali in http_tbl_alise:
                        if dedata.has_key(ali):
                            found = True
                            break
                    break
                if case():
                    ali = '__any__'
                    found = True
                    break

            if not found:
                ptr_tasks[task_id] = {'ret':1, 'result':'task id '+task_id + ' failed', 'error':'table error'}
                continue
            #1、修改数据库
            try:
                db_ret, go_on, old_data = worker.m_handlers[task_type][ali].callme(worker, dedata, ali, task_data['opt'])
            except Exception as e:
                g_req_loger.error(_lineno(), '!!!!--->', repr(e))
                g_req_loger.error(traceback.format_exc())
                #ptr_tasks[task_id] = {'ret':1, 'result':'task id '+task_id + ' failed', 'error':repr(e)}
                ptr_tasks[task_id] = {'ret':1, 'result':'task id '+task_id + ' failed', 'error':'set db failed!'}
                continue
            if not db_ret:
                ptr_tasks[task_id] = {'ret':1, 'result':'task id '+task_id + ' failed', 'error':'db error'}
                continue
            ptr_tasks[task_id] = {'ret':0, 'result':'task id '+task_id + ' succ', 'error':''}
            #2、如果有需要，通知proxy
            if go_on:
                try:
                    worker.m_handlers[task_type][ali].notify(worker, msgobj, opt=task_data['opt'], data=dedata, odata=old_data)
                except Exception as e:
                    g_req_loger.error(_lineno(), 'notify proxy error!!!!--->', repr(e))
                    g_req_loger.error(traceback.format_exc())
                    continue

        req_handler.notify_flush(worker, msgobj)
        worker.http_th.put(replymsg)

    @staticmethod
    def handle_inner_chk_task_done(http_th, data_done):
        data_done.pop('class')
        payload_encode= 'data='+json.dumps(data_done)
        g_req_loger.debug(_lineno(), 'post data:\n', repr(payload_encode))

        res, post_error = http_th.http_send_post(mgr_conf.g_url_inner_chk_task_ip,
                mgr_conf.g_url_inner_chk_task_url, payload_encode)
        if not res:
            g_req_loger.warn(traceback.format_exc())
            raise Exception(_lineno(), 'request task post code:', post_error)
        g_req_loger.debug(_lineno(), 'request task return:\n', repr(res))

        decodejson = json.loads(res)
        g_req_loger.debug(_lineno(), 'json ret:', repr(decodejson['ret']))
        g_req_loger.debug(_lineno(), 'json error:', repr(decodejson['error']))
        g_req_loger.debug(_lineno(), 'json result:\n', repr(decodejson['result']))


class req_hdl_abstract(object):
    loger = None
    handlers = []
    def __init__(self, loger):
        self.loger = loger

    def add(self, worker, data, ali_tbl):
        self.loger.error(_lineno(), 'not implement')
        raise Exception('opt not implement')

    def set(self, worker, data, ali_tbl):
        self.loger.error(_lineno(), 'not implement')
        raise Exception('opt not implement')

    def delete(self, worker, data, ali_tbl):
        self.loger.error(_lineno(), 'not implement')
        raise Exception('opt not implement')

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        self.loger.error(_lineno(), 'not implement')
        raise Exception('opt not implement')

    def callme(self, worker, data, ali_tbl, opt):
        for case in switch(opt):
            if case('add'):
                return self.add(worker, data, ali_tbl)
            if case('set'):
                return self.set(worker, data, ali_tbl)
            if case('del'):
                return self.delete(worker, data, ali_tbl)
            if case():
                self.loger.error(_lineno(), 'opt:', opt, ' not implement')
                raise Exception('opt not implement')

class req_handler_impl(req_hdl_abstract):
    def __init__(self, loger):
        req_hdl_abstract.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        #{"name":"go","A":"1.2.3.4","ttl":"10","viewid":"0"}
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record,
                (http_tbl_realname[ali_tbl], data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'],
                    data[ali_tbl], 0, n_enable, data['rid']))
        #result = worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        #if result[0][0]==None or result[0][1]==None:
        #    result = None
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])
            worker.dbcon.nextset()
            ars = worker.dbcon.show()
        worker.dbcon.fetch_proc_reset()
        self.loger.info(_lineno(), 'select old:', result)
        return add_ret, True, result

    def set(self, worker, data, ali_tbl):
        #'{"name":"t4.test.com","main":"test.com","rid":133,"A":"4.4.4.4","ttl":"10","viewid":"2"}
        #self.loger.info(_lineno(), 'updating name:', data['name'], ' table:', http_tbl_realname[ali_tbl], ' into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record,
                (http_tbl_realname[ali_tbl], data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'],
                    data[ali_tbl], 0, n_enable, data['rid']))
        #result = worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        #if result[0][0]==None or result[0][1]==None:
        #    result = None
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])
            worker.dbcon.nextset()
            ars = worker.dbcon.show()
        worker.dbcon.fetch_proc_reset()
        self.loger.info(_lineno(), 'select old:', result)
        return update_ret, True, result

    def delete(self, worker, data, ali_tbl):
        #{"rid":133,"A":"4.4.4.4"}
        #self.loger.info(_lineno(), 'deleting name:', http_tbl_realname[ali_tbl], ' rid:', data['rid'], ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        result = worker.dbcon.fetch_proc_reset()
        self.loger.info(_lineno(), 'select old:', result)
        if result[0][0]==None or result[0][1]==None:
            result = None
        return True, True, result

    def donotify(self, worker, msgobj, opt, data, odata, real_tbl):
        if len(worker.proxy_addr.keys()) < 1:
            return
        self.loger.debug(_lineno(), 'opt:', opt, ' data:', data)
        for case in switch(opt):
            if case('add'):
                #可以直接覆盖也就是发add
                if (odata and len(odata) > 0 and len(odata[0]) >= 4):
                    odata0 = odata[0]
                    ropt = 'del'
                    if odata0[3] > 0:
                        ropt = 'add'
                    msgobj.append({'opt':http_opt_str2int[ropt], 'domain':odata0[0], 'view':odata0[1], 'type':msg.g_dict_type[odata0[2]],
                            'pkt_head':msg.g_pack_head_init_dns})
                    req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

                # add: {u'A': u'9.9.9.9', u'viewid': u'2', u'main': u'test.com', u'name': u't9.test.com', u'ttl': u'10'}
                msgobj.append({'opt':http_opt_str2int['add'], 'domain':data['name'].lstrip('@.'), 'view':int(data['viewid']),
                    'type':msg.g_dict_type[real_tbl], 'pkt_head':msg.g_pack_head_init_dns})
                break
            if case('set'):
                if (odata and len(odata) > 0 and len(odata[0]) >= 4):
                    odata0 = odata[0]
                    ropt = 'del'
                    if odata0[3] > 0:
                        ropt = 'add'
                    msgobj.append({'opt':http_opt_str2int[ropt], 'domain':odata0[0], 'view':odata0[1], 'type':msg.g_dict_type[odata0[2]],
                            'pkt_head':msg.g_pack_head_init_dns})
                    req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

                # set: {"name":"t4.test.com","main":"test.com","rid":133,"A":"4.4.4.4","ttl":"10","viewid":"2"}
                ropt = 'add'
                if data.has_key('enable') and int(data['enable'])==0:
                    ropt = 'del'
                msgobj.append({'opt':http_opt_str2int[ropt], 'domain':data['name'].lstrip('@.'), 'view':int(data['viewid']),
                    'type':msg.g_dict_type[real_tbl], 'pkt_head':msg.g_pack_head_init_dns})
                break
            if case('del'):
                if (odata and len(odata) > 0 and len(odata[0]) >= 3):
                    odata0 = odata[0]
                    ropt = opt
                    if odata0[2] > 0:
                        ropt = 'add'
                    msgobj.append({'opt':http_opt_str2int[ropt], 'domain':odata0[0], 'view':odata0[1], 'type':msg.g_dict_type[real_tbl],
                        'pkt_head':msg.g_pack_head_init_dns})
                break
            if case():
                self.loger.warn(_lineno(), 'opt:', opt, ' has not been implemented!')

        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

class req_handler_record_a(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'adding name:', data['name'], ' table:a_record into database')
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'updating name:', data['name'], ' table:a_record into database')
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'deleting name:a_record rid:', data['rid'], ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        self.donotify(worker, msgobj, opt, data, odata, 'a_record')

class req_handler_record_aaaa(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'adding name:', data['name'], ' table:aaaa_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        self.loger.info(_lineno(), 'decode aaaa:', data[ali_tbl])
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'updating name:', data['name'], ' table:aaaa_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        self.loger.info(_lineno(), 'decode aaaa:', data[ali_tbl])
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'deleting name:aaaa_record rid:', data['rid'], ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        self.donotify(worker, msgobj, opt, data, odata, 'aaaa_record')

class req_handler_record_cname(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'adding name:', data['name'], ' table:cname_record into database')
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'updating name:', data['name'], ' table:cname_record into database')
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'deleting name:cname_record rid:', data['rid'], ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        self.donotify(worker, msgobj, opt, data, odata, 'cname_record')

class req_handler_record_ns(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'adding name:', data['name'], ' table:ns_record into database')
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'updating name:', data['name'], ' table:ns_record into database')
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'deleting name:ns_record rid:', data['rid'], ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        self.donotify(worker, msgobj, opt, data, odata, 'ns_record')

class req_handler_record_txt(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'adding name:', data['name'], ' table:txt_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl])) #.decode('UTF-8').encode("UTF-8")
        self.loger.info(_lineno(), 'decode TXT:', data[ali_tbl])
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'updating name:', data['name'], ' table:txt_record into database')
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl])) #.decode('UTF-8').encode("UTF-8")
        self.loger.info(_lineno(), 'decode TXT:', data[ali_tbl])
        return req_handler_impl.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'deleting name:txt_record rid:', data['rid'], ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        self.donotify(worker, msgobj, opt, data, odata, 'txt_record')

class req_handler_record_mx(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'adding name:', data['name'], ' table:mx_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_mx_record,
                (data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data['level'], data[ali_tbl],
                    0, n_enable, data['rid']))
        #result = worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        #if result[0][0]==None or result[0][1]==None:
        #    result = None
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])
            worker.dbcon.nextset()
            ars = worker.dbcon.show()
        worker.dbcon.fetch_proc_reset()
        self.loger.info(_lineno(), 'select old:', result)
        return add_ret, True, result

    def set(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'updating name:', data['name'], ' table:mx_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_mx_record,
                (data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data['level'], data[ali_tbl],
                    0, n_enable, data['rid']))
        #result = worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        #if result[0][0]==None or result[0][1]==None:
        #    result = None
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])
            worker.dbcon.nextset()
            ars = worker.dbcon.show()
        worker.dbcon.fetch_proc_reset()
        self.loger.info(_lineno(), 'select old:', result)
        return update_ret, True, result

    def delete(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'deleting name:mx_record rid:', data['rid'], ' from database')
        return req_handler_impl.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        self.donotify(worker, msgobj, opt, data, odata, 'mx_record')

class req_handler_record_domain_ns(req_hdl_abstract):
    def __init__(self, loger):
        req_hdl_abstract.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        #{u'opt': u'add', u'data': u'{"name":"@.ee.com","main":"ee.com","rid":324,"domain_ns":"ns2.dnspro.net.","level":"0","ttl":"600","viewid":"0"}', u'type': u'record'}
        self.loger.info(_lineno(), 'adding domain_ns:', data['main'], '-->', data['domain_ns'], ' into database')
        add_ret = worker.dbcon.query(msg.g_sql_add_a_domain_ns % (data['main'], int(data['ttl']), data['domain_ns'],
            data['rid'], data['main'], int(data['ttl']), data['domain_ns']))
        self.loger.debug(_lineno(), 'add return ', add_ret)
        return add_ret, False, None

    def set(self, worker, data, ali_tbl):
        #{u'opt': u'add', u'data': u'{"name":"@.ee.com","main":"ee.com","rid":324,"domain_ns":"ns2.dnspro.net.","level":"0","ttl":"600","viewid":"0"}', u'type': u'record'}
        self.loger.info(_lineno(), 'updating domain_ns:', data['main'], '-->', data['domain_ns'], ' into database')
        add_ret = worker.dbcon.query(msg.g_sql_add_a_domain_ns % (data['main'], int(data['ttl']), data['domain_ns'],
            data['rid'], data['main'], int(data['ttl']), data['domain_ns']))
        return add_ret, False, None

class req_handler_domain(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        return req_hdl_abstract.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        #{"name":"test.com", "enable":1/0}
        n_enable = 1 if int(data['enable'])==0 else 0
        self.loger.info(_lineno(), 'update domain:', data['name'], '[', n_enable, '] from database')
        worker.dbcon.call_proc(msg.g_proc_set_a_domain, (data['name'], n_enable))
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])
            worker.dbcon.nextset()
            ars = worker.dbcon.show()
        worker.dbcon.fetch_proc_reset()
        self.loger.info(_lineno(), 'select old:', result)
        return True, True, result

    def delete(self, worker, data, ali_tbl):
        if False:
            data['enable'] = 0
            return self.set(worker, data, ali_tbl)

        #{"name":"test.com"}
        self.loger.info(_lineno(), 'deleting domain:', data['name'], ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_domain, (data['name'],))
        result = []
        ars = worker.dbcon.show()
        while ars and len(ars) > 0:
            for i in range(len(ars)):
                result.append(ars[i])
            worker.dbcon.nextset()
            ars = worker.dbcon.show()
        worker.dbcon.fetch_proc_reset()
        self.loger.info(_lineno(), 'select old:', result)
        return True, True, result

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        self.donotify(worker, msgobj, opt, data, odata)

    def donotify(self, worker, msgobj, opt=None, data=None, odata=None, real_tbl=None):
        self.loger.debug(_lineno(), 'enter opt:', opt, ' data:', data, ' odata:', odata)
        if len(worker.proxy_addr.keys()) < 1:
            return
        for case in switch(opt):
            if case('del'):
                for od in odata:
                    if (len(od) >= 3 and od[0] and od[1] != None and od[2]):
                        self.loger.debug(_lineno(), 'notify od for del:', od)
                        msgobj.append({'opt':http_opt_str2int[opt], 'domain':od[0], 'view':od[1], 'type':msg.g_dict_type[od[2]],
                            'pkt_head':msg.g_pack_head_init_dns})
                        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])
                break
            if case('set'):
                ropt = 'add'
                if data.has_key('enable') and int(data['enable'])==0:
                    ropt = 'del'
                for od in odata:
                    if (len(od) >= 3 and od[0] and od[1] != None and od[2]):
                        self.loger.debug(_lineno(), 'notify od for set:', od)
                        msgobj.append({'opt':http_opt_str2int[ropt], 'domain':od[0], 'view':od[1], 'type':msg.g_dict_type[od[2]],
                            'pkt_head':msg.g_pack_head_init_dns})
                        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])
                break
            if case():
                self.loger.warn(_lineno(), 'opt:', opt, ' has not been implemented!')

class req_handler_view_mask(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger)

    def add(self, worker, data, ali_tbl):
        return req_hdl_abstract.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        return req_hdl_abstract.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        return req_hdl_abstract.delete(self, worker, data, ali_tbl)

    def notify(self, worker, msgobj, opt=None, data=None, odata=None, flush=False, direct=False):
        return req_hdl_abstract.notify(self, worker, msgobj, opt, data, odata, '', flush, direct)

http_tbl_alise = ('A', 'AAAA', 'CNAME', 'NS', 'TXT', 'MX', 'domain_ns')
http_tbl_realname = {'A' : 'a_record',
                     'AAAA' : 'aaaa_record',
                     'CNAME' : 'cname_record',
                     'NS' : 'ns_record',
                     'TXT' : 'txt_record',
                     'MX' : 'mx_record',
                     'domain_ns' : 'domain_ns'
                     }
http_opt_str2int = {'add':msg.g_opt_add, 'del':msg.g_opt_del}
http_type_to_proxy_header = {'record':msg.g_pack_head_init_dns, 'domain':msg.g_pack_head_init_view}

