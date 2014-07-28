#!/usr/bin/python
# -*- coding:UTF-8 -*-

import sys

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
