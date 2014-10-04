#!/usr/bin/python
# -*- coding:UTF-8 -*-
# made by likunxiang

class mgr_singleton(object):
    __slots__ = ('factory', 'loger', 'mgr_worker', 'mgr_worker4init', 'reply_th', 'http_th', 'chkchange_th',
            'zkhandler', 'err_info')
    def __init__(self, fact):
        self.factory = fact
        self.loger = None
        self.mgr_worker = None
        self.mgr_worker4init = None
        self.reply_th = None
        self.http_th = None
        self.chkchange_th = None
        self.zkhandler = None
        self.err_info = None

    def get_loger(self):
        if not self.loger:
            self.loger = self.factory.get_mgr_loger()
        return self.loger

    def get_worker4init(self):
        if not self.mgr_worker4init:
            self.mgr_worker4init = self.factory.get_mgr_worker4init(self.get_loger())
        return self.mgr_worker4init

    def get_worker(self):
        if not self.mgr_worker:
            self.mgr_worker = self.factory.get_mgr_worker(self.get_loger())
        return self.mgr_worker

    def get_reply_thread(self):
        if not self.reply_th:
            self.reply_th = self.factory.get_repth_thread(self.get_worker4init(), self.get_worker(), self.get_loger())
        return self.reply_th

    def get_http_thread(self):
        if not self.http_th:
            self.http_th = self.factory.get_http_thread(self.get_worker(), self.get_loger())
        return self.http_th

    def get_check_thread(self):
        if not self.chkchange_th:
            self.chkchange_th = self.factory.get_check_thread(self.get_worker4init(), self.get_worker(),
                    self.get_http_thread(), self.get_loger())
        return self.chkchange_th

    def get_zkhandler(self):
        if not self.zkhandler:
            self.zkhandler = self.factory.get_zkhandler(self.get_loger())
        return self.zkhandler

    def get_err_info(self):
        if not self.err_info:
            self.err_info = self.factory.get_err_info(self.get_loger())
        return self.err_info

g_singleton = None

