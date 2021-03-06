#!/usr/bin/python
#coding: utf-8
# made by likunxiang

import msg
import sys
import json
import types
import mgr_conf
from mgr_misc import _lineno, switch, partition_reserve_ip_from_ptr
import traceback
import urllib
import mgr_singleton
import time

__all__ = ['req_handler', 'req_handler_record_a', 'req_handler_record_ptr', 'req_handler_record_aaaa', 'req_handler_record_cname',
        'req_handler_record_ns', 'req_handler_record_txt', 'req_handler_record_mx', 'req_handler_record_domain_ns',
        'req_handler_domain', 'req_handler_view','req_handler_mask']

class req_handler(object):
    g_init_should_stop = 0
    @staticmethod
    def notify_proxy(worker, msgobj, addr, flush=False):
        if not flush and len(msgobj) >= mgr_conf.g_row_perpack or flush and len(msgobj) > 0:
            pre_pkt_head = None
            pre_i = 0
            for i in range(len(msgobj)):
                if msgobj[i].has_key('pkt_head'):
                    cur_pkt_head = msgobj[i].pop('pkt_head')
                else:
                    mgr_singleton.g_singleton.get_loger().error(_lineno(), 'no pkt_head key-->', repr(msgobj[i]))
                    cur_pkt_head = None
                if cur_pkt_head and (pre_pkt_head and cur_pkt_head != pre_pkt_head):
                    mgr_singleton.g_singleton.get_loger().info(_lineno(), 'pkt piece head:', pre_pkt_head, ' -->', repr(msgobj[pre_i:i]))
                    worker.reply(msgobj[pre_i:i], pre_pkt_head, addr)
                    pre_i = i
                if cur_pkt_head:
                    pre_pkt_head = cur_pkt_head

            if pre_pkt_head == None:
                mgr_singleton.g_singleton.get_loger().error(_lineno(), 'no pkt_head key-->', repr(msgobj))
            else:
                mgr_singleton.g_singleton.get_loger().care(_lineno(), 'pkt piece head:', pre_pkt_head, ' -->', repr(msgobj[pre_i:]))
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
        mgr_singleton.g_singleton.get_loger().info(_lineno(), 'set chk_init_time')

    @staticmethod
    def set_chk_snd_timer(worker):
        worker.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init)
        worker.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
        mgr_singleton.g_singleton.get_loger().info(_lineno(), 'set chk_snd_time')

    @staticmethod
    def handle_proxy_init_new(worker, addr):
        req_handler.g_init_should_stop = 0
        msg.g_init_sendstate = 1
        worker.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_snd)
        worker.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init)
        worker.dbcon.query(msg.g_sql_clean_snd_req)

        #msgobj = {'initing':1}
        #worker.sendto_(msgobj, addr, msg.g_pack_head_init_complete, mgr_conf.g_reply_port)

        #query dns
        msgobj = []
        count = 0
        cur_cnt = 0
        expect_cnt = 0

        #生成任务 record
        for atbl in msg.g_list_tbl:
            worker.dbcon.query(msg.g_init_sql_dns % (atbl))
            result = worker.dbcon.show()
            if not result:
                continue
            for row in result:
                worker.dbcon.call_proc(msg.g_proc_add_task, ('dns', msg.g_dict_type[atbl], row[1], row[0], 0, msg.g_opt_add))
        #发送一些先，防止收到多个init
        worker.dbcon.query(msg.g_init_sql_inittask_dns_limit1)
        result = worker.dbcon.show()
        if result:
            row = result[0]
            msg.g_init_maxid = int(row[0])
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            mgr_singleton.g_singleton.get_loger().care(_lineno(), "dns query %s res: %s" % (atbl, row))
            msgobj.append({'id':row[0],'opt':msg.g_opt_add, 'domain':row[3], 'view':row[2], 'type':row[1]})
            count += 1
            cur_cnt += 1
            expect_cnt += 1
            if worker.sendto_(msgobj, addr, msg.g_pack_head_init_dns, mgr_conf.g_reply_port) != True:
                return
            del msgobj[:]
            cur_cnt = 0
        msg.g_init_resp_expect = count
        mgr_singleton.g_singleton.get_loger().info(_lineno(), "sent first some %d records" % (count,))
        #生成任务 mask
        worker.dbcon.query(msg.g_init_sql_view)
        result = worker.dbcon.show()
        if result:
            for row in result:
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
            mgr_singleton.g_singleton.get_loger().error(_lineno(), "count task error open all timer")
            return

        mgr_singleton.g_singleton.get_loger().info(_lineno(), "g_init_should_stop %d" % (req_handler.g_init_should_stop,))

        #发送通知
        worker.dbcon.query(msg.g_init_sql_inittask_dns)
        result = worker.dbcon.show()
        if result:
            for row in result:
                worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
                mgr_singleton.g_singleton.get_loger().care(_lineno(), "dns query %s res: %s" % (atbl, row))
                msgobj.append({'id':row[0],'opt':msg.g_opt_add, 'domain':row[3], 'view':row[2], 'type':row[1]})
                count += 1
                if msg.g_init_sendstate == 1 and expect_cnt-count < 10:
                    msg.g_init_sendstate = 2
                    req_handler.set_chk_init_timer(worker)
                cur_cnt += 1
                if cur_cnt >= mgr_conf.g_row_perpack4init:
                    if req_handler.g_init_should_stop == 1: return
                    if worker.sendto_(msgobj, addr, msg.g_pack_head_init_dns, mgr_conf.g_reply_port) != True:
                        return
                    cur_cnt = 0
                    del msgobj[:]
                    time.sleep(1)

        if cur_cnt > 0:
            if req_handler.g_init_should_stop == 1: return
            if worker.sendto_(msgobj, addr, msg.g_pack_head_init_dns, mgr_conf.g_reply_port) != True:
                return
            time.sleep(1)
        mgr_singleton.g_singleton.get_loger().info(_lineno(), "sent %d records" % (count,))

        #query view
        del msgobj[:]
        cur_cnt = 0

        worker.dbcon.query(msg.g_init_sql_inittask_view)
        result = worker.dbcon.show()
        if result:
            for row in result:
                worker.dbcon.query(msg.g_init_sql_inittask_view_inited % (row[0],))
                mgr_singleton.g_singleton.get_loger().care(_lineno(), row)
                msgobj.append({'id':row[0], 'opt':msg.g_opt_add, 'view':row[1], 'mask':row[2]})
                count += 1
                if msg.g_init_sendstate == 1 and expect_cnt-count < 10:
                    msg.g_init_sendstate = 2
                    req_handler.set_chk_init_timer(worker)
                cur_cnt += 1
                if cur_cnt >= mgr_conf.g_row_perpack4init:
                    if req_handler.g_init_should_stop == 1: return
                    if worker.sendto_(msgobj, addr, msg.g_pack_head_init_view, mgr_conf.g_reply_port) != True:
                        return
                    cur_cnt = 0
                    del msgobj[:]
            if cur_cnt > 0:
                if req_handler.g_init_should_stop == 1: return
                if worker.sendto_(msgobj, addr, msg.g_pack_head_init_view, mgr_conf.g_reply_port) != True:
                    return

        if req_handler.g_init_should_stop == 1: return

        msgobj = {'complete':1}
        worker.sendto_(msgobj, addr, msg.g_pack_head_init_complete, mgr_conf.g_reply_port)
        msg.g_init_resp_expect = count
        #msg.g_init_complete = True

    @staticmethod
    def handle_proxy_init_reply(worker, answ, addr):
        str_class = answ['class'].partition('_')[0]
        state_set = 0
        if answ['type'] == msg.g_ptr_tbl:
            answ['data'] = partition_reserve_ip_from_ptr(answ['data'])
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
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), "tid:", answ['id'], ' ', str_class, ' ', answ['type'], ' ', answ['viewid'], ' ',
                answ['data'], ' ', state_set, ' ', answ['opt'])

        if worker.just4testcnt > 0:
            worker.dbcon.query(msg.g_init_sql_replytask % (state_set, answ['id']))

        if False and worker.just4testcnt > 0:
            for case in switch(str_class):
                if case('dns'):
                    mgr_singleton.g_singleton.get_err_info().del_record_timeout(answ['opt'], answ['viewid'], answ['data'], answ['type'])
                    break
                if case('view'):
                    mgr_singleton.g_singleton.get_err_info().del_view_timeout(answ['opt'], answ['viewid'], answ['data'])
                    break
                if case():
                    mgr_singleton.g_singleton.get_loger().warn(_lineno(), 'can not del error describe for unknow type ', str_class)

        if msg.g_init_sendstate == 2 and msg.g_init_maxid-int(answ['id']) < 10:
            req_handler.set_chk_snd_timer(worker)
            msg.g_init_sendstate = 0

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
        worker.dbcon.query(msg.g_inner_sql_chksnd_dns)
        result = worker.dbcon.show()
        if result:
            for row in result:
                worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
                msgobj.append({'id':row[0],'opt':row[4],'domain':row[3],'view':row[2],'type':row[1],
                    'pkt_head':msg.g_pack_head_init_dns})
                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0])
                if False:
                    mgr_singleton.g_singleton.get_err_info().add_record_timeout(row[4], row[2], row[3], row[1])
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
                    mgr_singleton.g_singleton.get_err_info().add_view_timeout(row[3], row[1], row[2])
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
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'post encode data:\n', repr(payload_encode))
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'test decode data:\n', repr(json.loads(payload_encode[5:])))
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'test pre ioopt--> utf8:', repr(payload['ioopt']))
        test_de = json.loads(payload_encode[5:])['ioopt']
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'test enc and dec ioopt:', test_de.encode("UTF-8"), ', utf8:', repr(test_de.encode("UTF-8")), ', unicode:', repr(test_de))

        res, post_error = http_th.http_send_post(mgr_conf.g_url_inner_chk_task_ip,
                mgr_conf.g_url_inner_chk_task_url, payload_encode)
        if not res:
            mgr_singleton.g_singleton.get_loger().warn(traceback.format_exc())
            raise Exception(_lineno(), 'request task post code:', post_error)
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'request task return:\n', repr(res))
        decodejson = json.loads(res)
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'json ret:', repr(decodejson['ret']))
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'json error:', repr(decodejson['error']))
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'json result:\n', repr(decodejson['result']))
        if decodejson['ret'] != 0:
            mgr_singleton.g_singleton.get_loger().warn(traceback.format_exc())
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
            mgr_singleton.g_singleton.get_loger().info(_lineno(), data['class'], ' no task')
            return

        replymsg = {'class': msg.g_class_inner_chk_task_done,
                'opt' : 'set',
                'data' : {
                    'sid':mgr_conf.g_mgr_sid,
                    'tasks':{} } }
        ptr_tasks = replymsg['data']['tasks']
        msgobj = []
        bat_task = []
        for task_id in dic_result:
            task_data = dic_result.get(task_id)
            mgr_singleton.g_singleton.get_loger().info(_lineno(), task_id, '\t:', task_data)

            try:
                dedata = json.loads(task_data['data'])
            except Exception,e:
                mgr_singleton.g_singleton.get_loger().error(_lineno(), 'load json data error!', repr(e))
                mgr_singleton.g_singleton.get_loger().error(traceback.format_exc())
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
                mgr_singleton.g_singleton.get_loger().error(_lineno(), '!!!!--->', repr(e))
                mgr_singleton.g_singleton.get_loger().error(traceback.format_exc())
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
                    is_bat_task = worker.m_handlers[task_type][ali].notify(worker, msgobj, opt=task_data['opt'], data=dedata, odata=old_data)
                    if is_bat_task:
                        dedata['handle_class'] = worker.m_handlers[task_type][ali]
                        bat_task.append(dedata)
                except Exception as e:
                    mgr_singleton.g_singleton.get_loger().error(_lineno(), 'notify proxy error!!!!--->', repr(e))
                    mgr_singleton.g_singleton.get_loger().error(traceback.format_exc())
                    continue

        req_handler.notify_flush(worker, msgobj)
        worker.http_th.put(replymsg)

        mgr_singleton.g_singleton.get_loger().care(_lineno(), 'bat tasks:', repr(bat_task))
        for dedata in bat_task:
            try:
                dedata['handle_class'].bat_notify(worker, dedata)
            except Exception as e:
                mgr_singleton.g_singleton.get_loger().error(_lineno(), 'handle bat task:', repr(dedata), '\n\terror!!!!--->', repr(e))
                mgr_singleton.g_singleton.get_loger().error(traceback.format_exc())
                continue

    @staticmethod
    def handle_inner_chk_task_done(http_th, data_done):
        data_done.pop('class')
        payload_encode= 'data='+json.dumps(data_done)
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'post data:\n', repr(payload_encode))

        res, post_error = http_th.http_send_post(mgr_conf.g_url_inner_chk_task_ip,
                mgr_conf.g_url_inner_chk_task_url, payload_encode)
        if not res:
            mgr_singleton.g_singleton.get_loger().warn(traceback.format_exc())
            raise Exception(_lineno(), 'request task post code:', post_error)
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'request task return:\n', repr(res))

        decodejson = json.loads(res)
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'json ret:', repr(decodejson['ret']))
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'json error:', repr(decodejson['error']))
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'json result:\n', repr(decodejson['result']))

    @staticmethod
    def handle_inner_chk_task_db_heartbeat(worker):
        worker.dbcon.query(msg.g_inner_sql_db_heartbeat)
        result = worker.dbcon.show()
        mgr_singleton.g_singleton.get_loger().care(_lineno(), repr(result))
        if not result:
            mgr_singleton.g_singleton.get_loger().warn(_lineno(), 'reconnecting to mysql!!!!!')
            worker.dbcon.query(msg.g_inner_sql_db_heartbeat)

    @staticmethod
    def handle_proxy_heartbeat(worker, data):
        objs = mgr_singleton.g_singleton.get_err_info().gen_msg()
        data['message'] = objs
        data['status'] = len(objs[0])>0 and -1 or 0
        mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'heatbeat payload:\n', repr(objs))
        worker.reply_echo(data, data['inner_addr'][0], data['inner_addr'][1])

class req_hdl_abstract(object):
    loger = None
    record_type = None
    def __init__(self, loger, rtype):
        self.loger = loger
        self.record_type = rtype

    def add(self, worker, data, ali_tbl):
        self.loger.error(_lineno(), self.record_type, ' not implement')
        raise Exception(self.record_type + ' add not implement')

    def set(self, worker, data, ali_tbl):
        self.loger.error(_lineno(), self.record_type, ' not implement')
        raise Exception(self.record_type + ' set not implement')

    def delete(self, worker, data, ali_tbl):
        self.loger.error(_lineno(), self.record_type, ' not implement')
        raise Exception(self.record_type + ' delete not implement')

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        self.loger.error(_lineno(), self.record_type, ' not implement')
        raise Exception(self.record_type + ' notify not implement')

    def bat_notify(self, worker, data):
        self.loger.error(_lineno(), self.record_type, ' not implement')
        raise Exception(self.record_type + ' bat_notify not implement')

    def callme(self, worker, data, ali_tbl, opt):
        for case in switch(opt):
            if case('add'):
                return self.add(worker, data, ali_tbl)
            if case('set'):
                return self.set(worker, data, ali_tbl)
            if case('del'):
                return self.delete(worker, data, ali_tbl)
            if case():
                self.loger.error(_lineno(), self.record_type, ' opt:', opt, ' not implement')
                raise Exception(self.record_type + ' opt:' + opt + ' not implement')

class req_handler_impl(req_hdl_abstract):
    def __init__(self, loger, rtype):
        req_hdl_abstract.__init__(self, loger, rtype)

    def add(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'adding name:', data['name'], ' table:', self.record_type, ' into database')
        #{"name":"go","A":"1.2.3.4","ttl":"10","viewid":"0"}
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record,
                (http_tbl_realname[ali_tbl], data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'],
                    data[ali_tbl], 0, n_enable, data['rid']))
        #result = []
        #ars = worker.dbcon.show()
        #while ars and len(ars) > 0:
        #    for i in range(len(ars)):
        #        result.append(ars[i])
        #    worker.dbcon.nextset()
        #    ars = worker.dbcon.show()
        #worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return add_ret, True, result

    def set(self, worker, data, ali_tbl):
        #'{"name":"t4.test.com","main":"test.com","rid":133,"A":"4.4.4.4","ttl":"10","viewid":"2"}
        self.loger.info(_lineno(), 'updating name:', data['name'], ' table:', self.record_type, ' into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_a_record,
                (http_tbl_realname[ali_tbl], data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'],
                    data[ali_tbl], 0, n_enable, data['rid']))
        #result = []
        #ars = worker.dbcon.show()
        #while ars and len(ars) > 0:
        #    for i in range(len(ars)):
        #        result.append(ars[i])
        #    worker.dbcon.nextset()
        #    ars = worker.dbcon.show()
        #worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return update_ret, True, result

    def delete(self, worker, data, ali_tbl):
        #{"rid":133,"A":"4.4.4.4"}
        self.loger.info(_lineno(), 'deleting name:', self.record_type, ' rid:', data['rid'], ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_record, (http_tbl_realname[ali_tbl], data['rid']))
        #result = worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        #if result[0][0]==None or result[0][1]==None:
        #    result = None
        #else:
        #    data['main'] = result[0][0]
        #    data['viewid'] = int(result[0][1])
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return True, True, result

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        return self.donotify(worker, msgobj, opt, data, odata, self.record_type)

    def donotify(self, worker, msgobj, opt, data, odata, real_tbl):
        if len(worker.proxy_addr.keys()) < 1:
            return False

        self.loger.debug(_lineno(), 'opt:', opt, ' odata:', odata)
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_dns_inited % (row[0],))
            msgobj.append({'opt':row[4], 'domain':row[3], 'view':row[2], 'type':row[1], 'id':row[0],
                'pkt_head':msg.g_pack_head_init_dns})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

        #self.loger.debug(_lineno(), 'opt:', opt, ' data:', data)
        #for case in switch(opt):
        #    if case('add'):
        #        #可以直接覆盖也就是发add
        #        if (odata and len(odata) > 0 and len(odata[0]) >= 4):
        #            odata0 = odata[0]
        #            ropt = 'del'
        #            if odata0[3] > 0:
        #                ropt = 'add'
        #            msgobj.append({'opt':http_opt_str2int[ropt], 'domain':odata0[0], 'view':odata0[1], 'type':msg.g_dict_type[odata0[2]],
        #                    'pkt_head':msg.g_pack_head_init_dns})
        #            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

        #        # add: {u'A': u'9.9.9.9', u'viewid': u'2', u'main': u'test.com', u'name': u't9.test.com', u'ttl': u'10'}
        #        msgobj.append({'opt':http_opt_str2int['add'], 'domain':data['name'].lstrip('@.'), 'view':int(data['viewid']),
        #            'type':msg.g_dict_type[real_tbl], 'pkt_head':msg.g_pack_head_init_dns})
        #        break
        #    if case('set'):
        #        if (odata and len(odata) > 0 and len(odata[0]) >= 4):
        #            odata0 = odata[0]
        #            ropt = 'del'
        #            if odata0[3] > 0:
        #                ropt = 'add'
        #            msgobj.append({'opt':http_opt_str2int[ropt], 'domain':odata0[0], 'view':odata0[1], 'type':msg.g_dict_type[odata0[2]],
        #                    'pkt_head':msg.g_pack_head_init_dns})
        #            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

        #        # set: {"name":"t4.test.com","main":"test.com","rid":133,"A":"4.4.4.4","ttl":"10","viewid":"2"}
        #        ropt = 'add'
        #        if data.has_key('enable') and int(data['enable'])==0:
        #            ropt = 'del'
        #        msgobj.append({'opt':http_opt_str2int[ropt], 'domain':data['name'].lstrip('@.'), 'view':int(data['viewid']),
        #            'type':msg.g_dict_type[real_tbl], 'pkt_head':msg.g_pack_head_init_dns})
        #        break
        #    if case('del'):
        #        if (odata and len(odata) > 0 and len(odata[0]) >= 3):
        #            odata0 = odata[0]
        #            ropt = opt
        #            if odata0[2] > 0:
        #                ropt = 'add'
        #            msgobj.append({'opt':http_opt_str2int[ropt], 'domain':odata0[0], 'view':odata0[1], 'type':msg.g_dict_type[real_tbl],
        #                'pkt_head':msg.g_pack_head_init_dns})
        #        break
        #    if case():
        #        self.loger.warn(_lineno(), 'opt:', opt, ' has not been implemented!')

        #req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        return False

class req_handler_record_a(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger, 'a_record')

class req_handler_record_ptr(req_handler_impl):  #跟A的处理差不多
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger, 'ptr_record')

    def add(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'adding ip:', data['name'], ' table:', self.record_type, ' into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_ptr_record,
                (data['name'], data[ali_tbl], int(data['viewid']), int(data['ttl']), 0, n_enable, int(data['rid'])) )
        #result = []
        #ars = worker.dbcon.show()
        #while ars and len(ars) > 0:
        #    for i in range(len(ars)):
        #        result.append(ars[i])
        #    worker.dbcon.nextset()
        #    ars = worker.dbcon.show()
        #worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return add_ret, True, result

    def set(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'updating ip:', data['name'], ' table:', self.record_type, ' into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_ptr_record,
                (data['name'], data[ali_tbl], int(data['viewid']), int(data['ttl']), 0, n_enable, int(data['rid'])) )
        #result = []
        #ars = worker.dbcon.show()
        #while ars and len(ars) > 0:
        #    for i in range(len(ars)):
        #        result.append(ars[i])
        #    worker.dbcon.nextset()
        #    ars = worker.dbcon.show()
        #worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return update_ret, True, result

class req_handler_record_aaaa(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger, 'aaaa_record')

    def add(self, worker, data, ali_tbl):
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        self.loger.info(_lineno(), 'decode aaaa:', data[ali_tbl])
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl]))
        self.loger.info(_lineno(), 'decode aaaa:', data[ali_tbl])
        return req_handler_impl.set(self, worker, data, ali_tbl)

class req_handler_record_cname(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger, 'cname_record')

    #def send1more(self, worker, msgobj, tblname, domain, view, ropt):
    #    if tblname == 'cname_record':
    #        mgr_singleton.g_singleton.get_loger().debug(_lineno(), "send 1 more A for CNAME : %s, opt : %s" % (domain, ropt))
    #        msgobj.append({'opt':http_opt_str2int[ropt], 'domain':domain, 'view':view, 'type':msg.g_dict_type['a_record'],
    #            'pkt_head':msg.g_pack_head_init_dns})
    #        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

    #def donotify(self, worker, msgobj, opt, data, odata, real_tbl):
    #    if len(worker.proxy_addr.keys()) < 1:
    #        return False
    #    self.loger.debug(_lineno(), 'opt:', opt, ' data:', data)
    #    for case in switch(opt):
    #        if case('add'):
    #            #可以直接覆盖也就是发add
    #            if (odata and len(odata) > 0 and len(odata[0]) >= 4):
    #                odata0 = odata[0]
    #                ropt = 'del'
    #                if odata0[3] > 0:
    #                    ropt = 'add'
    #                msgobj.append({'opt':http_opt_str2int[ropt], 'domain':odata0[0], 'view':odata0[1], 'type':msg.g_dict_type[odata0[2]],
    #                    'pkt_head':msg.g_pack_head_init_dns})
    #                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
    #                self.send1more(worker, msgobj, odata0[2], odata0[0], odata0[1], ropt)

    #            # add: {u'A': u'9.9.9.9', u'viewid': u'2', u'main': u'test.com', u'name': u't9.test.com', u'ttl': u'10'}
    #            msgobj.append({'opt':http_opt_str2int[opt], 'domain':data['name'].lstrip('@.'), 'view':int(data['viewid']),
    #                'type':msg.g_dict_type[real_tbl], 'pkt_head':msg.g_pack_head_init_dns})
    #            self.send1more(worker, msgobj, real_tbl, data['name'], int(data['viewid']), opt)
    #            break
    #        if case('set'):
    #            if (odata and len(odata) > 0 and len(odata[0]) >= 4):
    #                odata0 = odata[0]
    #                ropt = 'del'
    #                if odata0[3] > 0:
    #                    ropt = 'add'
    #                msgobj.append({'opt':http_opt_str2int[ropt], 'domain':odata0[0], 'view':odata0[1], 'type':msg.g_dict_type[odata0[2]],
    #                        'pkt_head':msg.g_pack_head_init_dns})
    #                req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
    #                self.send1more(worker, msgobj, odata0[2], odata0[0], odata0[1], ropt)

    #            # set: {"name":"t4.test.com","main":"test.com","rid":133,"A":"4.4.4.4","ttl":"10","viewid":"2"}
    #            ropt = 'add'
    #            if data.has_key('enable') and int(data['enable'])==0:
    #                ropt = 'del'
    #            msgobj.append({'opt':http_opt_str2int[ropt], 'domain':data['name'].lstrip('@.'), 'view':int(data['viewid']),
    #                'type':msg.g_dict_type[real_tbl], 'pkt_head':msg.g_pack_head_init_dns})
    #            self.send1more(worker, msgobj, real_tbl, data['name'], int(data['viewid']), ropt)
    #            break
    #        if case('del'):
    #            if (odata and len(odata) > 0 and len(odata[0]) >= 3):
    #                odata0 = odata[0]
    #                ropt = opt
    #                if odata0[2] > 0:
    #                    ropt = 'add'
    #                msgobj.append({'opt':http_opt_str2int[ropt], 'domain':odata0[0], 'view':odata0[1], 'type':msg.g_dict_type[real_tbl],
    #                    'pkt_head':msg.g_pack_head_init_dns})
    #                self.send1more(worker, msgobj, real_tbl, odata0[0], odata0[1], ropt)
    #            break
    #        if case():
    #            self.loger.warn(_lineno(), 'opt:', opt, ' has not been implemented!')

    #    req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
    #    return False

class req_handler_record_ns(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger, 'ns_record')

    def add_subrecord_inline(self, worker, str_main, n_vid, add_data):
        add_ret = worker.dbcon.call_proc(msg.g_proc_get_subrecord_inline, (str_main, n_vid))
        ars = worker.dbcon.show()
        self.loger.care(_lineno(), repr(ars))
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
                self.loger.care(_lineno(), repr(ars))
        worker.dbcon.fetch_proc_reset()
        return add_ret

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        req_handler_impl.notify(self, worker, msgobj, opt, data, odata)
        return True

    def bat_notify(self, worker, data):
        if len(worker.proxy_addr.keys()) < 1 or not data.has_key('main'):
            return
        self.loger.care(_lineno(), 'bat_data:', repr(data))
        sub_data = []
        sub_ret = self.add_subrecord_inline(worker, data['main'], int(data['viewid']), sub_data)
        self.loger.info(_lineno(), 'updating subrecord:', repr(sub_data))
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

class req_handler_record_txt(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger, 'txt_record')

    def add(self, worker, data, ali_tbl):
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl])) #.decode('UTF-8').encode("UTF-8")
        self.loger.info(_lineno(), 'decode TXT:', data[ali_tbl])
        return req_handler_impl.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        data[ali_tbl] = urllib.unquote(str(data[ali_tbl])) #.decode('UTF-8').encode("UTF-8")
        self.loger.info(_lineno(), 'decode TXT:', data[ali_tbl])
        return req_handler_impl.set(self, worker, data, ali_tbl)

class req_handler_record_mx(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger, 'mx_record')

    def add(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'adding name:', data['name'], ' table:', self.record_type, ' into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        add_ret = worker.dbcon.call_proc(msg.g_proc_add_mx_record,
                (data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data['level'], data[ali_tbl],
                    0, n_enable, data['rid']))
        #result = []
        #ars = worker.dbcon.show()
        #while ars and len(ars) > 0:
        #    for i in range(len(ars)):
        #        result.append(ars[i])
        #    worker.dbcon.nextset()
        #    ars = worker.dbcon.show()
        #worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return add_ret, True, result

    def set(self, worker, data, ali_tbl):
        self.loger.info(_lineno(), 'updating name:', data['name'], ' table:mx_record into database')
        n_enable = 1
        if data.has_key('enable'):
            n_enable = int(data['enable'])
        update_ret = worker.dbcon.call_proc(msg.g_proc_add_mx_record,
                (data['name'].lstrip('@.'), data['main'], data['viewid'], data['ttl'], data['level'], data[ali_tbl],
                    0, n_enable, data['rid']))
        #result = []
        #ars = worker.dbcon.show()
        #while ars and len(ars) > 0:
        #    for i in range(len(ars)):
        #        result.append(ars[i])
        #    worker.dbcon.nextset()
        #    ars = worker.dbcon.show()
        #worker.dbcon.fetch_proc_reset()
        #self.loger.info(_lineno(), 'select old:', result)
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return update_ret, True, result

class req_handler_record_domain_ns(req_hdl_abstract):
    def __init__(self, loger):
        req_hdl_abstract.__init__(self, loger, 'domain_ns')

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
        req_handler_impl.__init__(self, loger, 'domain')

    def add(self, worker, data, ali_tbl):
        return req_hdl_abstract.add(self, worker, data, ali_tbl)

    def set(self, worker, data, ali_tbl):
        #{"name":"test.com", "enable":1/0}
        n_enable = 1 if int(data['enable'])==0 else 0
        self.loger.info(_lineno(), 'update domain:', data['name'], '[', n_enable, '] from database')
        worker.dbcon.call_proc(msg.g_proc_set_a_domain, (data['name'], n_enable))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return True, True, result

    def delete(self, worker, data, ali_tbl):
        if False:
            data['enable'] = 0
            return self.set(worker, data, ali_tbl)

        for atbl in msg.g_list_tbl:
            #worker.dbcon.query(msg.g_sql_get_exist_records)
            worker.dbcon.query(msg.g_sql_get_exist_records % (atbl, data['name']))
            result = worker.dbcon.show()
            if result and len(result)>0:
                self.loger.error(_lineno(), 'deleting domain:', data['name'], ', find sub records:', repr(result), ' has sub records!!')
                return False, False, None
        #{"name":"test.com"}
        self.loger.info(_lineno(), 'deleting domain:', data['name'], ' from database')
        worker.dbcon.call_proc(msg.g_proc_del_a_domain, (data['name'],))
        worker.dbcon.query(msg.g_init_sql_gettask_dns)
        result = worker.dbcon.show()
        return True, True, result

class req_handler_view(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger, 'view')

    def add(self, worker, data, ali_tbl):
        data['vname'] = urllib.unquote(str(data['vname']))
        data['comment'] = urllib.unquote(str(data['comment']))
        data['ttl'] = int(data['ttl'])
        data['vid'] = int(data['vid'])
        self.loger.info(_lineno(), 'adding view name:', data['vname'], ' vid:', str(data['vid']),
                ' comment:', data['comment'], ' ttl:', str(data['ttl']))
        add_ret = worker.dbcon.query(msg.g_sql_add_a_view % (data['vid'], data['vname'], data['comment'],
            data['ttl'], data['comment'], data['ttl']))
        return add_ret, False, None

    def set(self, worker, data, ali_tbl):
        data['vname'] = urllib.unquote(str(data['vname']))
        data['comment'] = urllib.unquote(str(data['comment']))
        data['ttl'] = int(data['ttl'])
        data['vid'] = int(data['vid'])
        self.loger.info(_lineno(), 'updating view name:', data['vname'], ' vid:', str(data['vid']),
                ' comment:', data['comment'], ' ttl:', str(data['ttl']))
        add_ret = worker.dbcon.query(msg.g_sql_add_a_view % (data['vid'], data['vname'], data['comment'],
            data['ttl'], data['comment'], data['ttl']))
        return add_ret, False, None

    def delete(self, worker, data, ali_tbl):
        data['vid'] = int(data['vid'])
        self.loger.info(_lineno(), 'deleting view id:', str(data['vid']), ' from database')
        del_ret = worker.dbcon.query(msg.g_sql_del_a_view % (data['vid'],))
        return del_ret, False, None

    def notify(self, worker, msgobj, opt=None, data=None, odata=None):
        return req_hdl_abstract.notify(self, worker, msgobj, opt, data, odata)

class req_handler_mask(req_handler_impl):
    def __init__(self, loger):
        req_handler_impl.__init__(self, loger, 'mask')

    def add(self, worker, data, ali_tbl):
        #{“mask”:”123.150.107.0/24”,”vid”:2}
        data['vid'] = int(data['vid'])
        self.loger.info(_lineno(), 'adding mask mask:', data['mask'], ' vid:', str(data['vid']));
        add_ret = worker.dbcon.query(msg.g_sql_add_a_mask % (data['mask'], data['vid']), data['vid'])
        worker.dbcon.call_proc(msg.g_proc_add_task, ('view', 0, data['vid'], data['mask'], 0, msg.g_opt_add))
        worker.dbcon.query(msg.g_init_sql_gettask_mask)
        result = worker.dbcon.show()
        return add_ret, True, result

    def set(self, worker, data, ali_tbl):
        return req_hdl_abstract.set(self, worker, data, ali_tbl)

    def delete(self, worker, data, ali_tbl):
        #{“mask”:”123.150.107.0/24”,”vid”:2}
        data['vid'] = int(data['vid'])
        self.loger.info(_lineno(), 'deleting mask mask:', data['mask'], ' vid:', str(data['vid']));
        del_ret = worker.dbcon.query(msg.g_sql_del_a_mask % (data['mask'], data['vid']))
        worker.dbcon.call_proc(msg.g_proc_add_task, ('view', 0, data['vid'], data['mask'], 0, msg.g_opt_del))
        worker.dbcon.query(msg.g_init_sql_gettask_mask)
        result = worker.dbcon.show()
        return del_ret, True, result

    def donotify(self, worker, msgobj, opt, data, odata, real_tbl):
        if len(worker.proxy_addr.keys()) < 1:
            return False

        self.loger.debug(_lineno(), 'opt:', opt, ' odata:', odata)
        for row in odata:
            worker.dbcon.query(msg.g_init_sql_inittask_view_inited % (row[0],))
            msgobj.append({'id':row[0],'opt':row[3],'view':row[1],'mask':row[2],
                'pkt_head':msg.g_pack_head_init_view})
            req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)

        #self.loger.debug(_lineno(), 'opt:', opt, ' data:', data)
        #for case in switch(opt):
        #    if case('del') or case('add'):
        #        msgobj.append({'opt':http_opt_str2int[opt], 'view':data['vid'], 'mask':data['mask'],
        #            'pkt_head':msg.g_pack_head_init_view})
        #        break
        #    if case():
        #        self.loger.warn(_lineno(), 'opt:', opt, ' has not been implemented!')

        #req_handler.notify_proxy(worker, msgobj, worker.proxy_addr.keys()[0], False)
        return False

http_tbl_alise = ('A', 'PTR', 'AAAA', 'CNAME', 'NS', 'TXT', 'MX', 'domain_ns')
http_tbl_realname = {'A' : 'a_record',
                     'PTR' : 'ptr_record',
                     'AAAA' : 'aaaa_record',
                     'CNAME' : 'cname_record',
                     'NS' : 'ns_record',
                     'TXT' : 'txt_record',
                     'MX' : 'mx_record',
                     'domain_ns' : 'domain_ns'
                     }
http_opt_str2int = {'add':msg.g_opt_add, 'del':msg.g_opt_del}
http_type_to_proxy_header = {'record':msg.g_pack_head_init_dns, 'domain':msg.g_pack_head_init_view}
