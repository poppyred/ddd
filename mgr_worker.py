#!/usr/bin/env python
# -*- coding:UTF-8 -*-

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
from mgr_misc import _lineno
import traceback

class mgr_handler(queue_thread.Qthread):
    handler_qsize = 30000
    m_handlers = {}
    def __init__(self, loger):
        queue_thread.Qthread.__init__(self, 'mgr_work_thread', self.handler_qsize, loger)
        self.check_thd = None
        self.proxy_addr = {}
        self.dbip = mgr_conf.g_db_ip
        self.dbcon = MySQL.MySQL(host=self.dbip, user=mgr_conf.g_db_user, passwd=mgr_conf.g_db_passwd,
                db=mgr_conf.g_db_db, loger=loger)
        if self.dbcon.conn_error:
            self.loger.error(traceback.format_exc())
            raise Exception("[mgr_handler] Database configure error!!!")

        self.m_handlers['record'] = {}
        self.m_handlers['record']['A'] = req_handler_record_a(self.loger)
        self.m_handlers['record']['AAAA'] = req_handler_record_aaaa(self.loger)
        self.m_handlers['record']['CNAME'] = req_handler_record_cname(self.loger)
        self.m_handlers['record']['NS'] = req_handler_record_ns(self.loger)
        self.m_handlers['record']['TXT'] = req_handler_record_txt(self.loger)
        self.m_handlers['record']['MX'] = req_handler_record_mx(self.loger)
        self.m_handlers['record']['domain_ns'] = req_handler_record_domain_ns(self.loger)

        self.m_handlers['domain'] = {}
        self.m_handlers['domain']['__any__'] = req_handler_domain(self.loger)

        self.m_handlers['view_mask'] = {}
        self.m_handlers['view_mask']['__any__'] = req_handler_view_mask(self.loger)

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
            self.dbcon = MySQL.MySQL(host=self.dbip, loger=self.loger)
        try:
            self.loger.debug(_lineno(self), 'recv request class %s' % (data['class']))
            for case in switch(data['class']):
                if case(msg.g_class_init):
                    self.proxy_addr[data['inner_addr'][0]] = data['inner_addr']
                    msg.g_init_resp_expect = -1
                    req_handler.handle_proxy_init(self, data['inner_addr'][0])
                    break
                if case(msg.g_class_proxy_register):
                    self.proxy_addr[data['inner_addr'][0]] = data['inner_addr']
                    if msg.g_init_resp_expect == -1:
                        self.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init_ok)
                        self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
                        msg.g_init_resp_expect = 0
                        self.loger.info(_lineno(self), 'on register add timers OK')
                    break
                if case(msg.g_class_init_view_reply) or case(msg.g_class_init_dns_reply):
                    req_handler.handle_proxy_init_reply(self, data, data['inner_addr'][0])
                    break
                if case(msg.g_class_inner_chk_init_ok):
                    self.loger.info(_lineno(self), data['class'], '...')
                    ok_cnt = req_handler.handle_inner_chk_init_ok(self)
                    if ok_cnt != None and ok_cnt == msg.g_init_resp_expect:
                        self.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init_ok)
                        self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
                        self.loger.info(_lineno(self), 'on init add timers OK')
                    break
                if case(msg.g_class_inner_chk_snd):
                    self.loger.info(_lineno(self), data['class'], '...')
                    if len(self.proxy_addr.keys()) > 0:
                        req_handler.handle_inner_chk_snd(self)
                    else:
                        self.loger.warn(_lineno(self), 'proxy ip is empty')
                    break
                if case(msg.g_class_inner_chk_task_domain_reply) or case(msg.g_class_inner_chk_task_record_reply):
                    req_handler.handle_inner_chk_task_reply(self, data)
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
        self.loger.info(_lineno(self), 'sending:', encodedjson)
        str_fmt = "H" + str(len(encodedjson)) + "s"
        str_send = struct.pack(str_fmt, head, encodedjson)
        self.__sendto_short__(str_send, self.proxy_addr[addr][0])

    def __sendto_short__(self, data, host, port=12345):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((host, port))
            s.send(data)
        except socket.error, msg:
            self.loger.error(_lineno(self), 'dip(%s) (%s): %s' % (host, msg.args[0],msg.args[1]))
            self.loger.error(traceback.format_exc())
            if self.proxy_addr.has_key(host):
                self.proxy_addr.pop(host)
        finally:
            s.close()

    def onstop(self):
        self.m_handlers.clear()

