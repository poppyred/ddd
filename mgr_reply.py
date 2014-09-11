#!/usr/bin/env python
# -*- coding:UTF-8 -*-

import socket
import threading
import json
import msg
from request_handler import req_handler, switch
import struct
import sys
import Queue
import select
from mgr_misc import _lineno
import traceback

class reply_thread(threading.Thread):
    def __init__(self, worker4init, worker, loger, host='', port=54321, bufsize=1024):
        threading.Thread.__init__(self)
        self.HOST = host
        self.PORT = port
        self.udpSerSock = None
        self.BUFSIZE = bufsize
        self.thread_stop = False
        self.worker4init = worker4init
        self.worker = worker
        self.loger = loger

    def run(self): #Overwrite run() method, put what you want the thread do here
        ADDR = (self.HOST,self.PORT)
        self.udpSerSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udpSerSock.setblocking(False)
        self.udpSerSock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
        self.udpSerSock.bind(ADDR)
        rlists = [self.udpSerSock]
        while not self.thread_stop:
            try:
                rs,ws,es = select.select([self.udpSerSock], [], [], 1)
                if self.thread_stop:
                    break
                if not (rs or ws or es):
                    #timeout
                    continue
                for s in rs:
                    data, addr = s.recvfrom(self.BUFSIZE)
                    self.loger.info(_lineno(self), 'received from ', addr, ' data:', data)
                    if data==None:
                        self.loger.info(_lineno(self), 'recv data none')
                        continue
                    if len(data)==0:
                        self.loger.info(_lineno(self), 'recv data len 0')
                        continue
                    decodejson = json.loads(data)
                    decodejson['inner_addr'] = addr
                    for case in switch(decodejson['class']):
                        if case(msg.g_class_init_view_reply) or case(msg.g_class_init_dns_reply):
                            if msg.g_init_complete == True:
                                self.loger.info(_lineno(self), 'send to worker')
                                self.worker.put(decodejson)
                            else:
                                self.loger.info(_lineno(self), 'send to 4init')
                                self.worker4init.put(decodejson)
                            break
                        if case():
                            self.worker.put(decodejson)
            except Exception as e:
                self.loger.error(_lineno(self), 'inner error:', repr(e))
                self.loger.error(traceback.format_exc())
        self.udpSerSock.close()

    def stop(self):
        self.thread_stop = True
        udpCliSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        ADDR = ('localhost', self.PORT)
        udpCliSock.sendto("", ADDR)

