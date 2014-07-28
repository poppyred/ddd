#!/usr/bin/env python
# -*- coding:UTF-8 -*-

import queue_thread
import msg
import mgr_conf
import request_handler
import sys
import MySQL
import socket
import struct
import json
import time
from mgr_misc import _lineno
import traceback

class mgr_handler(queue_thread.Qthread):
    m_handlers = {}
    def __init__(self, loger):
        queue_thread.Qthread.__init__(self, 'mgr_work_thread', loger)
        self.proxy_addr = {}
        self.dbip = mgr_conf.g_db_ip
        self.dbcon = MySQL.MySQL(host=self.dbip, user=mgr_conf.g_db_user, passwd=mgr_conf.g_db_passwd,
                db=mgr_conf.g_db_db, loger=loger)
        if self.dbcon.conn_error:
            self.loger.error(traceback.format_exc())
            raise Exception("[mgr_handler] Database configure error!!!")

        self.m_handlers['record'] = {}
        self.m_handlers['record']['A'] = request_handler.req_handler_record_a(self.loger)
        self.m_handlers['record']['AAAA'] = request_handler.req_handler_record_aaaa(self.loger)
        self.m_handlers['record']['CNAME'] = request_handler.req_handler_record_cname(self.loger)
        self.m_handlers['record']['NS'] = request_handler.req_handler_record_ns(self.loger)
        self.m_handlers['record']['TXT'] = request_handler.req_handler_record_txt(self.loger)
        self.m_handlers['record']['MX'] = request_handler.req_handler_record_mx(self.loger)
        self.m_handlers['record']['domain_ns'] = request_handler.req_handler_record_domain_ns(self.loger)

        self.m_handlers['domain'] = {}
        self.m_handlers['domain']['__any__'] = request_handler.req_handler_domain(self.loger)

        self.m_handlers['view_mask'] = {}
        self.m_handlers['view_mask']['__any__'] = request_handler.req_handler_view_mask(self.loger)

        self.loger.info(_lineno(), 'handlers map:', repr(self.m_handlers))

        #self.__test__()

    def __test__(self):
        del_ret = self.dbcon.call_proc(msg.g_proc_del_a_record, ('a_record', 189))
        result = self.dbcon.fetch_proc_reset()
        print '1: delret:', del_ret, 'result:', result
        time.sleep(2)
        add_ret = self.dbcon.call_proc(msg.g_proc_add_a_record,
                ('a_record',  't3.test.com','test.com',2,10,'3.3.3.3',0,1,190))
        raa = self.dbcon.fetch_proc_reset()
        print '2: add_ret:', add_ret, 'result:', raa
        sys.exit()

    def set_http_thread(self, http_th):
        self.http_th = http_th

    def handler(self, data):
        if self.dbcon.conn_error:
            self.dbcon = MySQL.MySQL(host=self.dbip, loger=self.loger)
        try:
            self.loger.debug(_lineno(self), 'recv request class %s' % (data['class']))
            for case in request_handler.switch(data['class']):
                if case(msg.g_class_init):
                    self.proxy_addr[data['inner_addr'][0]] = data['inner_addr']
                    request_handler.req_handler.handle_proxy_init(self, data['inner_addr'][0])
                    break
                if case(msg.g_class_proxy_register):
                    self.proxy_addr[data['inner_addr'][0]] = data['inner_addr']
                    break
                if case(msg.g_class_init_view_reply) or case(msg.g_class_init_dns_reply):
                    request_handler.req_handler.handle_proxy_init_reply(self, data, data['inner_addr'][0])
                    break
                if case(msg.g_class_inner_chk_snd):
                    if len(self.proxy_addr.keys()) > 0:
                        request_handler.req_handler.handle_inner_chk_snd(self)
                    else:
                        self.loger.warn(_lineno(self), 'proxy ip is empty')
                    break
                if case(msg.g_class_inner_chk_task_domain_reply) or case(msg.g_class_inner_chk_task_record_reply):
                    request_handler.req_handler.handle_inner_chk_task_reply(self, data)
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

