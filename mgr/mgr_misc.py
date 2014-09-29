#!/usr/bin/python
# -*- coding:UTF-8 -*-
# made by likunxiang

import sys
import datetime

__all__ = ['_lineno']

def get_class(obj):
    return obj.__class__.__name__

def _lineno(obj=None):
    #print sys._getframe().f_back.f_code.co_filename.rpartition('/')
    fname = sys._getframe().f_back.f_code.co_filename
    if fname.find('/') != -1:
        fname = fname.rpartition('/')[2]
    if obj:
        return '{' + fname + ':' + str(sys._getframe().f_back.f_lineno) + '}[' + \
                get_class(obj) + ':' + sys._getframe().f_back.f_code.co_name + '] '
    else:
        return '{' + fname + ':' + str(sys._getframe().f_back.f_lineno) + '}[' + \
                sys._getframe().f_back.f_code.co_name + '] '

def get_time():
    now_tmp = datetime.datetime.now()
    str_times = now_tmp.strftime("%Y-%m-%d %H:%M:%S")
    return str_times

class switch(object):
    def __init__(self, value):
        self.value = value
        self.fall = False

    def __iter__(self):
        """Return the match method once, then stop"""
        yield self.match
        raise StopIteration

    def match(self, *args):
        """Indicate whether or not to enter a case suite"""
        if self.fall or not args:
            return True
        elif self.value in args: # changed for v1.5, see below
            self.fall = True
            return True
        else:
            return False

