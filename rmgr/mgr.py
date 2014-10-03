#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang


import time
import sys
import msg
import mgr_conf
from mgr_singleton import g_factory
import request_handler
import mgr_err_describe


def main_loop():
    while True:
        msg.g_now += 1
        time.sleep(1)
        g_factory.get_repth_thread().check_event()
        g_factory.get_check_thread().check_event()


if __name__ == '__main__':
    mgr_err_describe.g_err_desc = g_factory.get_err_info()
    g_factory.get_repth_thread()
    g_factory.get_http_thread()
    g_factory.get_mgr_worker().set_buddy_thread(g_factory.get_http_thread(), g_factory.get_check_thread())
    g_factory.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_db_heartbeat,
            mgr_conf.g_inner_chk_task_db_heartbeat)
    time.sleep(1)
    main_loop()

