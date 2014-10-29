#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import mgr_reply
import http_thread
import thread_chkchange
import mgr_worker
import mgr_worker4init
import mgr_conf
import mgr_err_describe

class mgr_factory(object):
    __slots__ = ('mgr_worker', 'mgr_worker4init', 'reply_th', 'http_th', 'chkchange_th', 'err_info')

    def __init__(self):
        self.reply_th = None
        self.http_th = None
        self.chkchange_th = None
        self.mgr_worker = None
        self.mgr_worker4init = None
        self.err_info = None

    def get_mgr_worker4init(self):
        if not self.mgr_worker4init:
            self.mgr_worker4init = mgr_worker4init.mgr_handler4init()
        return self.mgr_worker4init

    def get_mgr_worker(self):
        if not self.mgr_worker:
            self.mgr_worker = mgr_worker.mgr_handler()
        return self.mgr_worker

    def get_repth_thread(self):
        if not self.reply_th:
            self.reply_th = mgr_reply.reply_thread(self.get_mgr_worker4init(), self.get_mgr_worker(), port=mgr_conf.g_listen_port)
        return self.reply_th

    def get_http_thread(self):
        if not self.http_th:
            self.http_th = http_thread.http_thread(self.get_mgr_worker())
        return self.http_th

    def get_check_thread(self):
        if not self.chkchange_th:
            self.chkchange_th = thread_chkchange.thread_chkchange(self.get_mgr_worker4init(), self.get_mgr_worker(), self.get_http_thread())
        return self.chkchange_th

    def get_err_info(self):
        if not self.err_info:
            self.err_info = mgr_err_describe.ErrInfo()
        return self.err_info

g_factory = mgr_factory()
