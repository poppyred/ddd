#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import msg
import mgr_conf
from request_handler import *
import sys
import MySQL
import time

class mgr_handler4init(object):

    def __init__(self):
        self.dbip = mgr_conf.g_db_ip
        self.dbcon = MySQL.MySQL(self.dbip, mgr_conf.g_db_user, mgr_conf.g_db_passwd, mgr_conf.g_db_db)
        if self.dbcon.conn_error:
            raise Exception('[mgr_handler4init] Database configure error!!!')
        self.just4testcnt = 0

    def handler(self, data):
        if self.dbcon.conn_error:
            self.dbcon = MySQL.MySQL(self.dbip, mgr_conf.g_db_user, mgr_conf.g_db_passwd, mgr_conf.g_db_db)
        try:
            print >> sys.stderr,  'recv request class %s' % data['class']
            if data['class'] == msg.g_class_init_view_reply or data['class'] == msg.g_class_init_dns_reply:
                self.just4testcnt += 1
                req_handler.handle_proxy_init_reply(self, data, data['inner_addr'][0])
            if data['class'] == msg.g_class_inner_chk_task_db_heartbeat:
                req_handler.handle_inner_chk_task_db_heartbeat(self)

        except Exception as e:
            print >> sys.stderr,  ('inner error: ' + repr(e))
