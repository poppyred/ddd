#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import threading
import time
import mgr_worker
import http_thread
import socket
import msg
import sys

class task_node:
    def __init__(self, name, interval):
        self.nname = name
        self.interval = interval
        self.deadline = msg.g_now + interval

class thread_chkchange(object):

    def __init__(self, workq4init, workq, httpq):
        self.thread_stop = False
        self.tasknodeds = []
        self.lock = threading.Lock()
        self.tq = workq
        self.tq4init = workq4init
        self.http_tq = httpq

    def add_tasknode_byinterval_lock(self, name, interval):
        self.add_tasknode_lock(task_node(name, interval))

    def add_tasknode_lock(self, node):
        self.lock.acquire()
        self.add_tasknode(node)
        self.lock.release()

    def add_tasknode(self, node):
        self.del_tasknode_byname(node.nname)
        added = False
        for i in range(len(self.tasknodeds)):
            if self.tasknodeds[i].deadline < node.deadline:
                self.tasknodeds.insert(i, node)
                added = True
                break

        if not added:
            print ('add tasknodeds-->' + repr(self.tasknodeds))
            self.tasknodeds.append(node)
        print ('after add tasknodeds-->' + repr(self.tasknodeds))

    def del_tasknode_byname_lock(self, name):
        self.lock.acquire()
        self.del_tasknode_byname(name)
        self.lock.release()

    def del_tasknode_byname(self, name):
        for i in range(len(self.tasknodeds) - 1, -1, -1):
            if self.tasknodeds[i].nname == name:
                del self.tasknodeds[i]

    def check_event(self):
        self.lock.acquire()
        print ('tasknodeds***-->' + repr(self.tasknodeds))
        for pos in range(len(self.tasknodeds)):
            if self.tasknodeds[pos].deadline <= msg.g_now:
                newtns = self.tasknodeds[pos:]
                del self.tasknodeds[pos:]
                print ('newtns-->', repr(newtns))
                for timeout in newtns:
                    msgobj = {'class': timeout.nname}
                    try:
                        if timeout.nname == msg.g_class_inner_chk_task_domain or timeout.nname == msg.g_class_inner_chk_task_record:
                            self.http_tq.handler(msgobj)

                        elif timeout.nname == msg.g_class_inner_chk_task_db_heartbeat:
                            self.tq.handler(msgobj)
                            self.tq4init.handler(msgobj)

                        else:
                            self.tq.handler(msgobj)

                    except Exception as e:
                        print ('error taskq is ', repr(e))
                    finally:
                        timeout.deadline = msg.g_now + timeout.interval
                        self.add_tasknode(timeout)

                del newtns[:]
                break

        self.lock.release()

