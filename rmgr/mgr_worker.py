#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import msg
import mgr_conf
from request_handler import *
import sys
import MySQL
import socket
import struct
import json
import time

class mgr_handler(object):
    proxy_health = 3
    m_handlers = {}

    def __init__(self):
        self.check_thd = None
        self.proxy_addr = {}
        self.dbip = mgr_conf.g_db_ip
        self.dbcon = MySQL.MySQL(self.dbip, mgr_conf.g_db_user, mgr_conf.g_db_passwd, mgr_conf.g_db_db)
        if self.dbcon.conn_error:
            raise Exception('[mgr_handler] Database configure error!!!')
        self.m_handlers['record'] = {}
        self.m_handlers['record']['A'] = req_handler_record_a()
        self.m_handlers['record']['PTR'] = req_handler_record_ptr()
        self.m_handlers['record']['AAAA'] = req_handler_record_aaaa()
        self.m_handlers['record']['CNAME'] = req_handler_record_cname()
        self.m_handlers['record']['NS'] = req_handler_record_ns()
        self.m_handlers['record']['TXT'] = req_handler_record_txt()
        self.m_handlers['record']['MX'] = req_handler_record_mx()
        self.m_handlers['record']['domain_ns'] = req_handler_record_domain_ns()
        self.m_handlers['domain'] = {}
        self.m_handlers['domain']['__any__'] = req_handler_domain()
        self.m_handlers['view_mask'] = {}
        self.m_handlers['view_mask']['__any__'] = req_handler_view_mask()
        print >> sys.stderr,  ('handlers map:'+ repr(self.m_handlers))

    def set_buddy_thread(self, http_th, check_thd):
        self.http_th = http_th
        self.check_thd = check_thd

    def handler(self, data):
        if self.dbcon.conn_error:
            self.dbcon = MySQL.MySQL(self.dbip, mgr_conf.g_db_user, mgr_conf.g_db_passwd, mgr_conf.g_db_db)
        try:
            print >> sys.stderr,  'recv request class %s' % data['class']

            if data['class'] == msg.g_class_init_test:
                self.check_thd.del_tasknode_byname_lock(msg.g_class_init_test)
                self.proxy_addr['121.201.12.66'] = [('121.201.12.66', 12353), self.proxy_health]
                msg.g_init_resp_expect = -1
                req_handler.handle_proxy_init_new(self, '121.201.12.66')

            if data['class'] == msg.g_class_proxy_register:
                print >> sys.stderr,  (data['class'] + '... expect[%d]' % (msg.g_init_resp_expect,))
                self.proxy_addr[data['inner_addr'][0]] = [data['inner_addr'], self.proxy_health]
                if msg.g_init_resp_expect == -1:
                    self.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init_ok)
                    self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
                    self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_domain, mgr_conf.g_inner_chk_task_domain_time)
                    self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_record, mgr_conf.g_inner_chk_task_record_time)
                    msg.g_init_resp_expect = 0
                    print >> sys.stderr,  'on register add timers OK'

            if data['class'] == msg.g_class_inner_chk_init_ok:
                ok_cnt = req_handler.handle_inner_chk_init_ok(self)
                if ok_cnt:
                    print >> sys.stderr,  (data['class'] + '... expect[' + str(msg.g_init_resp_expect) + '][' + str(ok_cnt) + ']')
                else:
                    print >> sys.stderr,  (data['class'] + '... expect[' + str(msg.g_init_resp_expect) + '][NULL]')
                if ok_cnt != None and ok_cnt == msg.g_init_resp_expect:
                    self.check_thd.del_tasknode_byname_lock(msg.g_class_inner_chk_init_ok)
                    self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
                    self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_domain, mgr_conf.g_inner_chk_task_domain_time)
                    self.check_thd.add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_record, mgr_conf.g_inner_chk_task_record_time)
                    print >> sys.stderr,  'on init add timers OK'

            if data['class'] == msg.g_class_inner_chk_snd or data['class'] == msg.g_class_inner_chk_init:
                print >> sys.stderr,  (str(data['class']) + '...')
                del_items = []
                for k, v in self.proxy_addr.iteritems():
                    v[1] = v[1] - 1
                    print >> sys.stderr,  'proxy %s->%s[%d]' % (k, v, v[1])
                    if v[1] <= 0:
                        del_items.append(k)

                for p in del_items:
                    self.proxy_addr.pop(p)

                if len(self.proxy_addr.keys()) > 0:
                    req_handler.handle_inner_chk_snd(self)
                else:
                    print >> sys.stderr,  'proxy ip is empty'

            if data['class'] == msg.g_class_inner_chk_task_domain_reply or data['class'] == msg.g_class_inner_chk_task_record_reply:
                req_handler.handle_inner_chk_task_reply(self, data)

            if data['class'] == msg.g_class_inner_chk_task_db_heartbeat:
                req_handler.handle_inner_chk_task_db_heartbeat(self)

            if data['class'] == msg.g_class_proxy_heartbeat:
                req_handler.handle_proxy_heartbeat(self, data)

        except Exception as e:
            print >> sys.stderr,  ('inner error: ' + repr(e))

    def reply_echo(self, data, host, port):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            addre = (host, port)
            encodedjson = json.dumps(data)
            s.sendto(encodedjson, addre)
        except socket.error as msg:
            print >> sys.stderr,  'dip(%s) (%s): %s' % (host, msg.args[0], msg.args[1])
        finally:
            s.close()

    def reply(self, msgobj, head, addr):
        if addr == None or not self.proxy_addr.has_key(addr):
            print >> sys.stderr,  ('addr is error!!!! addr: ' + repr(addr))
            return
        encodedjson = json.dumps(msgobj)
        print >> sys.stderr,  ('sending:' + str(encodedjson))
        str_fmt = 'H' + str(len(encodedjson)) + 's'
        str_send = struct.pack(str_fmt, head, encodedjson)
        self.__sendto_short__(str_send, self.proxy_addr[addr][0][0], mgr_conf.g_reply_port)

    def __sendto_short__(self, data, host, port = 12345):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            s.connect((host, port))
            s.send(data)
        except socket.error as msg:
            print >> sys.stderr,  'dip(%s) (%s): %s' % (host, msg.args[0], msg.args[1])
            if self.proxy_addr.has_key(host):
                self.proxy_addr.pop(host)
        finally:
            s.close()

    def sendto_(self, msgobj, addr, head, port = 12345):
        if addr == None or not self.proxy_addr.has_key(addr):
            print >> sys.stderr,  ('addr is error!!!! addr: ' + repr(addr))
            return False
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            host = self.proxy_addr[addr][0][0]
            encodedjson = json.dumps(msgobj)
            str_fmt = 'H' + str(len(encodedjson)) + 's'
            str_send = struct.pack(str_fmt, head, encodedjson)
            s.connect((host, port))
            cnt = len(str_send)
            print >> sys.stderr,  'need to send(%d)' % (cnt,)
            ibegin = 0
            iend = 0
            ilen = 0
            while True:
                ilen = mgr_conf.g_size_perpack if cnt > mgr_conf.g_size_perpack else cnt
                iend += ilen
                data = str_send[ibegin:iend]
                s.send(data)
                cnt -= ilen
                print >> sys.stderr,  'sent(%d)' % (ilen,)
                ibegin = iend
                if cnt <= 0:
                    break

            return True
        except socket.error as msg:
            print >> sys.stderr,  'dip(%s) (%s): %s' % (host, msg.args[0], msg.args[1])
            if self.proxy_addr.has_key(addr):
                self.proxy_addr.pop(addr)
            return False
        finally:
            s.close()

