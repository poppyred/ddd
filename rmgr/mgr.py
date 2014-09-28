#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang


import time
import sys
import msg
import mgr_conf
from mgr_singleton import g_factory
import request_handler

def stop_all():
    if g_factory.get_check_thread().isAlive():
        g_factory.get_check_thread().stop()
        g_factory.get_check_thread().join()
    if g_factory.get_http_thread().isAlive():
        g_factory.get_http_thread().stop()
        g_factory.get_http_thread().join()
    if g_factory.get_repth_thread().isAlive():
        g_factory.get_repth_thread().stop()
        g_factory.get_repth_thread().join()
    if g_factory.get_mgr_worker4init().isAlive():
        g_factory.get_mgr_worker4init().stop()
        g_factory.get_mgr_worker4init().join()
    if g_factory.get_mgr_worker().isAlive():
        g_factory.get_mgr_worker().stop()
        g_factory.get_mgr_worker().join()


def main_loop():
    while 1:
        msg.g_now += 1
        time.sleep(1)

    stop_all()


if __name__ == '__main__':
    g_factory.get_repth_thread()
    g_factory.get_http_thread()
    g_factory.get_mgr_worker().set_buddy_thread(g_factory.get_http_thread(), g_factory.get_check_thread())
    g_factory.get_mgr_worker4init().start()
    g_factory.get_mgr_worker().start()
    g_factory.get_repth_thread().start()
    time.sleep(1)
    g_factory.get_http_thread().start()
    g_factory.get_check_thread().start()
    main_loop()
