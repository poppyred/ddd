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

class reply_thread(object):

    def __init__(self, worker4init, worker, host = '', port = 54321, bufsize = 1024):
        self.HOST = host
        self.PORT = port
        self.udpSerSock = None
        self.BUFSIZE = bufsize
        self.worker4init = worker4init
        self.worker = worker

        ADDR = (self.HOST, self.PORT)
        self.udpSerSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udpSerSock.setblocking(False)
        self.udpSerSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.udpSerSock.bind(ADDR)

    def check_event(self):
        try:
            rs, ws, es = select.select([self.udpSerSock], [], [], 1)
            if not (rs or ws or es):
                return
            for s in rs:
                data, addr = s.recvfrom(self.BUFSIZE)
                print ('received from ', addr, ' data:', data)
                if data == None:
                    print 'recv data none'
                    continue
                if len(data) == 0:
                    print 'recv data len 0'
                    continue
                decodejson = json.loads(data)
                decodejson['inner_addr'] = addr
                if decodejson['class'] == msg.g_class_init_view_reply or decodejson['class'] == msg.g_class_init_dns_reply:
                    self.worker4init.handler(decodejson)
                else:
                    self.worker.handler(decodejson)

        except Exception as e:
            print ('inner error:', repr(e))

        #self.udpSerSock.close()

