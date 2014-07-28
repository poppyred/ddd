#!/usr/bin/python
# -*- coding:UTF-8 -*-

import threading
import Queue
import sys
from mgr_misc import _lineno
import traceback

__all__ = ['Qthread']

class Qthread(threading.Thread):
    def __init__(self, name, loger):
        threading.Thread.__init__(self)
        self.thread_stop = False
        self.th_name = name
        self.tq = Queue.Queue(1000)
        self.loger = loger
        self.log_queue = False

    def get_queue():
        return self.tq

    def put(self, data):
        self.tq.put(data, block=False)

    def run(self):
        while not self.thread_stop:
            #self.loger.debug(_lineno(self), '[', self.th_name, '] waiting for message...')
            try:
                data = self.tq.get(block=True, timeout=1)
                if self.thread_stop:
                    self.tq.task_done()
                    break
                self.handler(data)
                self.tq.task_done()
            except Queue.Empty as e:
                if self.log_queue:
                    self.loger.debug(_lineno(self), '[', self.th_name, ':run] taskq is ', repr(e))
                    self.loger.debug(traceback.format_exc())
            except Exception as e:
                self.tq.task_done()
                self.loger.error(_lineno(self), '[', self.th_name, ':run] inner error:', repr(e))
                self.loger.error(traceback.format_exc())
        self.onstop()

    def handler(self, data):
        pass

    def stop(self):
        self.thread_stop = True

    def onstop(self):
        pass

