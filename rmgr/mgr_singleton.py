# 2014.09.28 17:31:11 CST
#Embedded file name: ./mgr_singleton.py
import mgr_reply
import http_thread
import thread_chkchange
import mgr_worker
import mgr_worker4init
import mgr_conf
import zk_handler
__all__ = ['g_factory']

class mgr_factory(object):
    __slots__ = ('mgr_worker', 'mgr_worker4init', 'reply_th', 'http_th', 'chkchange_th', 'zkhandler')

    def __init__(self):
        self.reply_th = None
        self.http_th = None
        self.chkchange_th = None
        self.mgr_worker = None
        self.mgr_worker4init = None
        self.zkhandler = None

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
            self.chkchange_th = thread_chkchange.thread_chkchange(self.get_mgr_worker4init().tq, self.get_mgr_worker().tq, self.get_http_thread().tq)
        return self.chkchange_th

    def get_zkhandler(self):
        if not self.zkhandler:
            self.zkhandler = zk_handler.LKXZookeeper()
        return self.zkhandler


g_factory = mgr_factory()
+++ okay decompyling mgr_singleton.pyc 
# decompiled 1 files: 1 okay, 0 failed, 0 verify failed
# 2014.09.28 17:31:11 CST
