#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import socket
import json
import msg
from request_handler import req_handler
import struct
import sys
import select
import threading
import mgr_conf
import MySQL

class handle_init_thread(threading.Thread):
    def __init__(self, json_data, worker):
        threading.Thread.__init__(self)
        self.dbip = mgr_conf.g_db_ip
        self.dbcon = MySQL.MySQL(self.dbip, mgr_conf.g_db_user, mgr_conf.g_db_passwd, mgr_conf.g_db_db)
        self.json_data = json_data
        self.worker = worker
        self.check_thd = worker.check_thd
        if self.dbcon.conn_error:
            raise Exception('[handle_init_thread] Database configure error!!!')

    def run(self):
        try:
            print >> sys.stderr,  'recv request class %s' % self.json_data['class']
            if self.json_data['class'] == msg.g_class_init:
                #self.proxy_addr[data['inner_addr'][0]] = [data['inner_addr'], self.proxy_health]
                #msg.g_init_resp_expect = -1
                #mgr_conf.g_row_perpack = mgr_conf.g_row_perpack4init
                req_handler.handle_proxy_init_new(self, self.json_data['inner_addr'][0])
        except Exception as e:
            print >> sys.stderr,  ('inner error: ' + repr(e))

    def sendto_(self, msgobj, addr, head, port = 12345):
        if addr == None or not self.worker.proxy_addr.has_key(addr):
            print >> sys.stderr,  ('addr is error!!!! addr: ' + repr(addr))
            return False
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            host = self.worker.proxy_addr[addr][0][0]
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
            return False
        finally:
            s.close()


class reply_thread(threading.Thread):
    def __init__(self, worker4init, worker, host = '', port = 54321, bufsize = 1024):
        threading.Thread.__init__(self)
        self.thread_stop = False
        self.HOST = host
        self.PORT = port
        self.udpSerSock = None
        self.BUFSIZE = bufsize
        self.worker4init = worker4init
        self.worker = worker

    def run(self):
        ADDR = (self.HOST, self.PORT)
        self.udpSerSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udpSerSock.setblocking(False)
        self.udpSerSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.udpSerSock.bind(ADDR)
        while not self.thread_stop:
            try:
                rs, ws, es = select.select([self.udpSerSock], [], [], 1)
                if self.thread_stop:
                    break
                if not (rs or ws or es):
                    continue
                for s in rs:
                    data, addr = s.recvfrom(self.BUFSIZE)
                    print >> sys.stderr,  ('received from ' + str(addr) + ' data:' + str(data))
                    if data == None:
                        print >> sys.stderr,  'recv data none'
                        continue
                    if len(data) == 0:
                        print >> sys.stderr,  'recv data len 0'
                        continue
                    decodejson = json.loads(data)
                    decodejson['inner_addr'] = addr
                    if decodejson['class'] == msg.g_class_init_view_reply or decodejson['class'] == msg.g_class_init_dns_reply:
                        self.worker4init.handler(decodejson)
                    elif decodejson['class'] == msg.g_class_init:
                        print >> sys.stderr,  '*********1'
                        self.worker.proxy_addr[decodejson['inner_addr'][0]] = [decodejson['inner_addr'], self.worker.proxy_health]
                        mgr_conf.g_row_perpack = mgr_conf.g_row_perpack4init
                        hdl_init = handle_init_thread(decodejson, self.worker)
                        print >> sys.stderr,  '*********3'
                        hdl_init.start()
                    else:
                        self.worker.handler(decodejson)

            except Exception as e:
                print >> sys.stderr,  ('inner error:' + repr(e))
        self.udpSerSock.close()

    def stop(self):
        self.thread_stop = True
