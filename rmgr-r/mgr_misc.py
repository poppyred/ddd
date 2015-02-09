#!/usr/bin/python
# -*- coding:UTF-8 -*-
# made by likunxiang

import sys
import datetime

def get_time():
    now_tmp = datetime.datetime.now()
    str_times = now_tmp.strftime("%Y-%m-%d %H:%M:%S")
    return str_times

def partition_reserve_ip_from_ptr(str_data):
    str_data = str_data.partition('.in-')
    #print repr(str_data)

    if len(str_data) < 1:
        return None
    str_data = str_data[0].split(".")
    split_len = len(str_data)
    str_ret = "";
    for i in range(split_len-1,-1,-1):
        if i != 0:
            str_ret += str_data[i] + "."
        else:
            str_ret += str_data[i]
    #print repr(str_ret)
    return str_ret
