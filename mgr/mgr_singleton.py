#!/usr/bin/python
# -*- coding:UTF-8 -*-
# made by likunxiang

import mgr_reply
import http_thread
import thread_chkchange
import mgr_worker
import mgr_worker4init
import mgr_loger
import mgr_conf
import zk_handler
import mgr_err_describe

__all__ = ['g_factory']

class mgr_factory(object):
    __slots__ = ('loger', 'mgr_worker', 'mgr_worker4init', 'reply_th', 'http_th', 'chkchange_th',
            'zkhandler', 'err_info')
    def __init__(self):
        self.reply_th = None
        self.http_th = None
        self.chkchange_th = None
        self.mgr_worker = None
        self.mgr_worker4init = None
        self.loger = None
        self.zkhandler = None
        self.err_info = None

    def get_mgr_loger(self):
        if not self.loger:
            self.loger = mgr_loger.mgr_loger(mgr_conf.log_level,
                    mgr_conf.log_direction, mgr_conf.log_path, mgr_conf.log_name)
            self.loger.setloger(self.loger)
        return self.loger

    def get_mgr_worker4init(self):
        if not self.mgr_worker4init:
            self.mgr_worker4init = mgr_worker4init.mgr_handler4init(self.get_mgr_loger())
        return self.mgr_worker4init

    def get_mgr_worker(self):
        if not self.mgr_worker:
            self.mgr_worker = mgr_worker.mgr_handler(self.get_mgr_loger())
        return self.mgr_worker

    def get_repth_thread(self):
        if not self.reply_th:
            self.reply_th = mgr_reply.reply_thread(self.get_mgr_worker4init(), self.get_mgr_worker(),
                    self.get_mgr_loger(), port=mgr_conf.g_listen_port)
        return self.reply_th

    def get_http_thread(self):
        if not self.http_th:
            self.http_th = http_thread.http_thread(self.get_mgr_worker(), self.get_mgr_loger())
        return self.http_th

    def get_check_thread(self):
        if not self.chkchange_th:
            #self.chkchange_th = thread_chkchange.thread_chkchange('localhost',reply_th.PORT)
            self.chkchange_th = thread_chkchange.thread_chkchange(
                    self.get_mgr_worker4init().tq,
                    self.get_mgr_worker().tq,
                    self.get_http_thread().tq,
                    self.get_mgr_loger())
        return self.chkchange_th

    def get_zkhandler(self):
        if not self.zkhandler:
            self.zkhandler = zk_handler.LKXZookeeper(self.get_mgr_loger())
        return self.zkhandler

    def get_err_info(self):
        if not self.err_info:
            self.err_info = mgr_err_describe.ErrInfo(self.get_mgr_loger())
        return self.err_info

g_factory = mgr_factory()
