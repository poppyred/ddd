#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import threading
import msg
import time

class Tick_thread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.thread_stop = False

    def run(self):
        while not self.thread_stop:
            msg.g_now += 1
            time.sleep(1)

    def stop(self):
        self.thread_stop = True
