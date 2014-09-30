#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import queue_thread
import msg
import mgr_conf
from request_handler import *
import sys
import MySQL
import time
from mgr_misc import _lineno, switch
import traceback

class mgr_handler4init(queue_thread.Qthread):
    handler_qsize = 40000
    def __init__(self, loger):
        queue_thread.Qthread.__init__(self, 'mgr_work_thread4init', self.handler_qsize, loger)
        self.dbip = mgr_conf.g_db_ip
        self.dbcon = MySQL.MySQL(self.dbip, mgr_conf.g_db_user, mgr_conf.g_db_passwd,
                mgr_conf.g_db_db, loger=loger)
        if self.dbcon.conn_error:
            self.loger.error(traceback.format_exc())
            raise Exception("[mgr_handler4init] Database configure error!!!")

        self.just4testcnt = 0

        #self.__test__()

    def __test__(self):
        if False:
            del_ret = self.dbcon.call_proc(msg.g_proc_del_a_record, ('a_record', 189))
            result = self.dbcon.fetch_proc_reset()
            print '1: delret:', del_ret, 'result:', result
            time.sleep(2)
            add_ret = self.dbcon.call_proc(msg.g_proc_add_a_record,
                    ('a_record',  't3.test.com','test.com',2,10,'3.3.3.3',0,1,190))
            raa = self.dbcon.fetch_proc_reset()
            print '2: add_ret:', add_ret, 'result:', raa

        if True:
            self.dbcon.query(msg.g_init_sql_chk_init_ok)
            result = self.dbcon.show()
            print repr(result)
            print result[0][0]

        sys.exit()

    def handler(self, data):
        if self.dbcon.conn_error:
            self.dbcon = MySQL.MySQL(self.dbip, mgr_conf.g_db_user, mgr_conf.g_db_passwd,
                    mgr_conf.g_db_db, loger=self.loger)
        try:
            self.loger.care(_lineno(self), 'recv request class %s' % (data['class']))
            for case in switch(data['class']):
                if case(msg.g_class_init_view_reply) or case(msg.g_class_init_dns_reply):
                    self.just4testcnt += 1
                    req_handler.handle_proxy_init_reply(self, data, data['inner_addr'][0])
                    break
                if case(msg.g_class_inner_chk_task_db_heartbeat):
                    req_handler.handle_inner_chk_task_db_heartbeat(self)
                    break
                if case():
                    self.loger.warn(_lineno(self), 'recv something else: ', data['class'])
        except Exception as e:
            self.loger.error(_lineno(self), 'inner error: ', repr(e))
            self.loger.error(traceback.format_exc())

