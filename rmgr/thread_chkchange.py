# 2014.09.28 17:32:07 CST
#Embedded file name: ./thread_chkchange.py
import threading
import time
import mgr_worker
import http_thread
import socket
import msg
import Queue
import sys
from request_handler import switch

class task_node:

    def __init__(self, name, interval):
        self.nname = name
        self.interval = interval
        self.deadline = msg.g_now + interval


class thread_chkchange(threading.Thread):

    def __init__(self, workq4init, workq, httpq):
        threading.Thread.__init__(self)
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
            self.tasknodeds.append(node)

    def del_tasknode_byname_lock(self, name):
        self.lock.acquire()
        self.del_tasknode_byname(name)
        self.lock.release()

    def del_tasknode_byname(self, name):
        for i in range(len(self.tasknodeds) - 1, -1, -1):
            if self.tasknodeds[i].nname == name:
                del self.tasknodeds[i]

    def run(self):
        self.thread_stop = False
        while not self.thread_stop:
            time.sleep(1)
            if self.thread_stop:
                break
            self.lock.acquire()
            for pos in range(len(self.tasknodeds)):
                if self.tasknodeds[pos].deadline <= msg.g_now:
                    newtns = self.tasknodeds[pos:]
                    del self.tasknodeds[pos:]
                    for timeout in newtns:
                        msgobj = {'class': timeout.nname}
                        try:
                            for case in switch(timeout.nname):
                                if case(msg.g_class_inner_chk_task_domain) or case(msg.g_class_inner_chk_task_record):
                                    self.http_tq.put(msgobj, block=False)
                                    break
                                if case(msg.g_class_inner_chk_task_db_heartbeat):
                                    self.tq.put(msgobj, block=False)
                                    self.tq4init.put(msgobj, block=False)
                                    break
                                if case():
                                    self.tq.put(msgobj, block=False)

                        except Queue.Full as e:
                            print ('taskq is ', repr(e))
                        finally:
                            timeout.deadline = msg.g_now + timeout.interval
                            self.add_tasknode(timeout)

                    del newtns[:]
                    break

            self.lock.release()

        del self.tasknodeds[:]

    def stop(self):
        self.thread_stop = True
+++ okay decompyling thread_chkchange.pyc 
# decompiled 1 files: 1 okay, 0 failed, 0 verify failed
# 2014.09.28 17:32:07 CST
