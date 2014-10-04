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
    def __init__(self):
        pass

    def get_mgr_loger(self):
        loger = mgr_loger.mgr_loger(mgr_conf.log_level,
                mgr_conf.log_direction, mgr_conf.log_path, mgr_conf.log_name)
        loger.setloger(loger)
        return loger

    def get_mgr_worker4init(self, loger):
        worker4init = mgr_worker4init.mgr_handler4init(loger)
        return worker4init

    def get_mgr_worker(self, loger):
        worker = mgr_worker.mgr_handler(loger)
        return worker

    def get_repth_thread(self, worker4init, worker, loger):
        reply_th = mgr_reply.reply_thread(worker4init, worker, loger, port=mgr_conf.g_listen_port)
        return reply_th

    def get_http_thread(self, worker, loger):
        http_th = http_thread.http_thread(worker, loger)
        return http_th

    def get_check_thread(self, worker4init, worker, http_th, loger):
        chkchange_th = thread_chkchange.thread_chkchange(worker4init.tq, worker.tq, http_th.tq, loger)
        return chkchange_th

    def get_zkhandler(self, loger):
        zkhandler = zk_handler.LKXZookeeper(loger)
        return zkhandler

    def get_err_info(self, loger):
        err_info = mgr_err_describe.ErrInfo(loger)
        return err_info

g_factory = mgr_factory()

