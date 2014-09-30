#!/usr/bin/python
# -*- coding:UTF-8 -*-
# made by likunxiang

import sys
import datetime

def get_time():
    now_tmp = datetime.datetime.now()
    str_times = now_tmp.strftime("%Y-%m-%d %H:%M:%S")
    return str_times

