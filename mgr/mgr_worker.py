#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import queue_thread
import msg
import mgr_conf
from request_handler import *
import sys
import MySQL
import socket
import struct
import json
import time
from mgr_misc import _lineno, switch
import traceback

class mgr_handler(queue_thread.Qthread):
    handler_qsize = 40000
    proxy_health = 3
    m_handlers = {}
    def __init__(self, loger):
        queue_thread.Qthread.__init__(self, 'mgr_work_thread', self.handler_qsize, loger)
        self.check_thd = None
        self.proxy_addr = {}
        self.dbip = mgr_conf.g_db_ip
        self.dbcon = MySQL.MySQL(self.dbip, mgr_conf.g_db_user, mgr_conf.g_db_passwd,
                mgr_conf.g_db_db, loger=loger)
        if self.dbcon.conn_error:
            self.loger.error(traceback.format_exc())
            raise Exception("[mgr_handler] Database configure error!!!")

        self.m_handlers['record'] = {}
        self.m_handlers['record']['A'] = req_handler_record_a(self.loger)
        self.m_handlers['record']['PTR'] = req_handler_record_ptr(self.loger)
        self.m_handlers['record']['AAAA'] = req_handler_record_aaaa(self.loger)
        self.m_handlers['record']['CNAME'] = req_handler_record_cname(self.loger)
        self.m_handlers['record']['NS'] = req_handler_record_ns(self.loger)
        self.m_handlers['record']['TXT'] = req_handler_record_txt(self.loger)
        self.m_handlers['record']['MX'] = req_handler_record_mx(self.loger)
        self.m_handlers['record']['domain_ns'] = req_handler_record_domain_ns(self.loger)

        self.m_handlers['domain'] = {}
        self.m_handlers['domain']['__any__'] = req_handler_domain(self.loger)

        self.m_handlers['view'] = {}
        self.m_handlers['view']['__any__'] = req_handler_view(self.loger)

        self.m_handlers['mask'] = {}
        self.m_handlers['mask']['__any__'] = req_handler_mask(self.loger)

        self.loger.info(_lineno(), 'handlers map:', repr(self.m_handlers))

        #self.__test__()

    def __test__(self):
        if False:
            del_ret = self.dbcon.call_proc(msg.g_proc_del_a_record, ('a_record', 189))
            result = self.dbcon.fetch_proc_reset()
            print '1: delret:', del_ret, 'result:', result
            time.sleep(2)
            add_ret = self.dbcon.call_proc(msg.g_proc_add_a_record,
                    ('a_record',  't3.test.com','test.com',2,10,'3.3.3.3',0,1,190))
            raa = self.dbcon.fetch_proc_reset()
            print '2: add_ret:', add_ret, 'result:', raa

        if True:
            self.dbcon.query(msg.g_init_sql_chk_init_ok)
            result = self.dbcon.show()
            print repr(result)
            print result[0][0]

        sys.exit()

    def set_buddy_thread(self, http_th, check_thd):
        self.http_th = http_th
        self.check_thd = check_thd

    def handler(self, data):
        if self.dbcon.conn_error:
            self.dbcon = MySQL.MySQL(self.dbip, mgr_conf.g_db_user, mgr_conf.g_db_passwd,
                    mgr_conf.g_db_db, loger=self.loger)
        try:
            self.loger.info(_lineno(self), 'recv request class %s' % (data['class']))
            for case in switch(data['class']):
                if case(msg.g_class_init):
                    self.proxy_addr[data['inner_addr'][0]] = [data['inner_addr'], self.proxy_health]
                    msg.g_init_resp_expect = -1
                    #msg.g_init_complete = False
                    req_handler.g_init_should_stop = 1
                    self.loger.info(_lineno(self), 'set g_init_should_stop %d' % (req_handler.g_init_should_stop,))
                    req_handler.handle_proxy_init_new(self, data['inner_addr'][0])
                    break
                if case(msg.g_class_init_test):
                    self.check_thd.del_tasknode_byname_lock(msg.g_class_init_test)
                    self.proxy_addr['121.201.12.66'] = [('121.201.12.66', 12353), self.proxy_health]
                    msg.g_init_resp_expect = -1
                    #msg.g_init_complete = False
                    req_handler.g_init_should_stop = 1
                    req_handler.handle_proxy_init_new(self, '121.201.12.66')
                    break
                if case(msg.g_class_proxy_register):
                    self.loger.info(_lineno(self), data['class'], '... expect[', msg.g_init_resp_expect, ']')
                    self.proxy_addr[data['inner_addr'][0]] = [data['inner_addr'], self.proxy_health]
                    if msg.g_init_resp_expect == -1:
                        self.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init_ok)
                        self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
                        self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_domain, mgr_conf.g_inner_chk_task_domain_time)
                        self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_record, mgr_conf.g_inner_chk_task_record_time)
                        msg.g_init_resp_expect = 0
                        #msg.g_init_complete = True
                        self.loger.info(_lineno(self), 'on register add timers OK')
                    break
                if case(msg.g_class_inner_chk_init_ok):
                    ok_cnt = req_handler.handle_inner_chk_init_ok(self)
                    if ok_cnt:
                        self.loger.info(_lineno(self), data['class'], '... expect[', msg.g_init_resp_expect, '][', ok_cnt,']')
                    else:
                        self.loger.info(_lineno(self), data['class'], '... expect[', msg.g_init_resp_expect, '][NULL]')
                    if ok_cnt != None and ok_cnt == msg.g_init_resp_expect:
                        self.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init_ok)
                        self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
                        self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_domain, mgr_conf.g_inner_chk_task_domain_time)
                        self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_record, mgr_conf.g_inner_chk_task_record_time)
                        #msg.g_init_complete = True
                        self.loger.info(_lineno(self), 'on init add timers OK')
                    break
                if case(msg.g_class_inner_chk_snd) or case(msg.g_class_inner_chk_init):
                    self.loger.info(_lineno(self), data['class'], '...')
                    del_items = []
                    for k, v in self.proxy_addr.iteritems():
                        v[1] = v[1] - 1
                        self.loger.info(_lineno(self), 'proxy %s->%s[%d]' % (k, v, v[1]))
                        if v[1] <= 0:
                            del_items.append(k)
                    for p in del_items:
                        self.proxy_addr.pop(p)

                    if len(self.proxy_addr.keys()) > 0:
                        req_handler.handle_inner_chk_snd(self)
                    else:
                        self.loger.warn(_lineno(self), 'proxy ip is empty')
                    break
                if case(msg.g_class_inner_chk_task_domain_reply) or case(msg.g_class_inner_chk_task_record_reply):
                    req_handler.handle_inner_chk_task_reply(self, data)
                    break
                if case(msg.g_class_inner_chk_task_db_heartbeat):
                    req_handler.handle_inner_chk_task_db_heartbeat(self)
                    break
                if case():
                    self.loger.warn(_lineno(self), 'recv something else: ', data['class'])
        except Exception as e:
            self.loger.error(_lineno(self), 'inner error: ', repr(e))
            self.loger.error(traceback.format_exc())

    def reply(self, msgobj, head, addr):
        if addr == None or not self.proxy_addr.has_key(addr):
            self.loger.error(_lineno(self), 'addr is error!!!! addr: ', repr(addr))
            return
        encodedjson = json.dumps(msgobj)
        self.loger.care(_lineno(self), 'sending:', encodedjson)
        str_fmt = "H" + str(len(encodedjson)) + "s"
        str_send = struct.pack(str_fmt, head, encodedjson)
        self.__sendto_short__(str_send, self.proxy_addr[addr][0][0], mgr_conf.g_reply_port)

    def __sendto_short__(self, data, host, port=12345):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((host, port))
            s.send(data)
        except socket.error, msg:
            self.loger.error(_lineno(self), 'dip(%s) (%s): %s' % (host, msg.args[0],msg.args[1]))
            self.loger.error(traceback.format_exc())
            if self.proxy_addr.has_key(host):
                self.proxy_addr.pop(host)
        finally:
            s.close()

    def sendto_(self, msgobj, addr, head, port=12345):
        if addr == None or not self.proxy_addr.has_key(addr):
            self.loger.error(_lineno(self), 'addr is error!!!! addr: ', repr(addr))
            return False
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            host = self.proxy_addr[addr][0][0]
            encodedjson = json.dumps(msgobj)
            str_fmt = "H" + str(len(encodedjson)) + "s"
            str_send = struct.pack(str_fmt, head, encodedjson)

            s.connect((host, port))
            cnt = len(str_send)
            self.loger.info(_lineno(self), 'need to send(%d)' % (cnt,))
            ibegin = 0
            iend = 0
            ilen = 0
            while True:
                ilen = mgr_conf.g_size_perpack if cnt>mgr_conf.g_size_perpack else cnt
                iend += ilen
                data = str_send[ibegin:iend]
                s.send(data)
                cnt -= ilen
                self.loger.info(_lineno(self), 'sent(%d)' % (ilen,))
                ibegin = iend
                if cnt <= 0:
                    break
            return True
        except socket.error, msg:
            self.loger.error(_lineno(self), 'dip(%s) (%s): %s' % (host, msg.args[0],msg.args[1]))
            self.loger.error(traceback.format_exc())
            if self.proxy_addr.has_key(addr):
                self.proxy_addr.pop(addr)
            return False
        finally:
            s.close()

    def onstop(self):
        self.m_handlers.clear()

