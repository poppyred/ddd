#!/usr/bin/python
# -*- coding:UTF-8 -*-

import threading
import queue_thread
import sys
import types
import datetime
from mgr_misc import _lineno
import traceback
import os
import msg

class mgr_loger(queue_thread.Qthread):
    __slots__ = ('ldebug', 'lcare', 'linfo', 'lwarn', 'lerror', 'dic_level')
    loger_qsize = 10000
    maxline = msg.g_log_maxline
    ldebug = 4
    lcare = 3
    linfo = 2
    lwarn = 1
    lerror = 0
    dic_level = {'error':lerror, 'warn':lwarn, 'info':linfo, 'care':lcare, 'debug':ldebug}
    dic_levelstr = {lerror:'<ERROR>', lwarn:'<WARN>', linfo:'<INFO>', lcare:'<CARE>', ldebug:'<DEBUG>'}

    def __init__(self, strlevel, _dir=0, lpath=None, lname=None):
        queue_thread.Qthread.__init__(self, 'mgr_loger', self.loger_qsize, None)
        self.lock = threading.Lock()
        self.realtime = False
        self.level = self.ldebug
        self.strlevel = 'debug'
        self.set_level(strlevel)
        self.str_times = ['', '']
        self.linec = 0
        now_tmp = datetime.datetime.now() #这是时间数组格式
        #转换为指定的格式
        self.str_times[0] = now_tmp.strftime("%Y-%m-%d %H:%M:%S")
        self.time_idx = 0

        self.dir_ = _dir
        if self.dir_==1:
            self.fp = None
            self.lpath = lpath.strip()
            while len(self.lpath) > 1 and self.lpath[-1]=='/':
                self.lpath = self.lpath[:-1]

            if lname.find('.') != -1 :
                self.postfix = '.' + lname.rpartition('.')[2]
                self.lname = lname.rpartition('.')[0]
            else:
                self.postfix = '.log'
                self.lname = lname
            self.fname = self.lpath + '/' + self.lname + self.postfix
            if os.path.isfile(self.fname):
                self._renamef()

    def _renamef(self):
        os.rename(self.fname, self.lpath + '/' + self.lname + '_' + \
                self.str_times[self.time_idx] + self.postfix)

    def _closef(self):
        if self.dir_ == 1 and self.fp != None:
            self.fp.close()
            self.fp = None
            self.linec = 0
            self._renamef()

    def _openf(self):
        try:
            os.makedirs(self.lpath)
            self._closef()
            self.fp = open(self.fname, "wb")
        except OSError, e:
            #print repr(e)
            if e[0]==17:
                self._closef()
                self.fp = open(self.fname, "wb")
                #print '222'
            else:
                print traceback.format_exc()
                raise e

    def _fflush(self):
        if self.dir_==1:
            if self.fp:
                self.fp.flush()
        else:
            sys.stdout.flush()

    def _w2file(self, data):
        if self.dir_ == 1 and self.linec > self.maxline:
            self._closef()
        if self.dir_ == 1 and self.fp == None:
            self._openf()
        try:
            if self.dir_ == 1:
                self.fp.write(data)
            else:
                sys.stdout.write(data)
        except Exception as e:
            sys.stderr.write(self.dic_levelstr[self.lerror] + '(' + self.str_times[self.time_idx] + ')' + _lineno(self) \
                    + '\n' + traceback.format_exc())
            self._closef()

    def __wlog__(self, l, ldata):
        dlen = len(ldata)
        if dlen > 0 and l <= self.level:
            self._w2file(self.dic_levelstr[l] + '(' + self.str_times[self.time_idx] + ')')
            #sys.stdout.write(self.dic_levelstr[l] + '(' + self.str_times[self.time_idx] + ')')
            for x in range(0, dlen):
                if type(x) != types.StringType:
                    self._w2file(str(ldata[x]))
                    #sys.stdout.write(str(ldata[x]))
                if x+1 == dlen:
                    self._w2file('\n')
                    self.linec = self.linec + 1
                    #sys.stdout.write('\n')
            self._fflush()

    def __chmod__(self, l, ldata):
        if not self.realtime:
            return True
        if l > self.level:
            return False
        try:
            self.lock.acquire()
            self.__wlog__(l, ldata)
        except Exception as e:
            sys.stderr.write('<ERROR>(%s)%s error:%s\n' % (self.str_times[self.time_idx], _lineno(self), e))
            sys.stderr.write(traceback.format_exc())
        finally:
            self.lock.release()
            return False
        return False

    def __chklog__(self, l, ldata):
        if self.__chmod__(l, ldata):
            self.put({'level':l, 'log':ldata})

    def get_level(self):
        return self.strlevel

    def set_level(self, strl):
        if not self.dic_level.has_key(strl):
            return
        self.level = self.dic_level[strl]
        self.strlevel = strl
        if self.level == self.ldebug:
            self.realtime = True

    def error(self, *ldata):
        self.__chklog__(self.dic_level['error'], ldata)

    def warn(self, *ldata):
        self.__chklog__(self.dic_level['warn'], ldata)

    def info(self, *ldata):
        self.__chklog__(self.dic_level['info'], ldata)

    def care(self, *ldata):
        self.__chklog__(self.dic_level['care'], ldata)

    def debug(self, *ldata):
        self.__chklog__(self.dic_level['debug'], ldata)

    def handler(self, data):
        self.__wlog__(data['level'], data['log'])

    def onstop(self):
        self._closef()

    def _uptime(self):
        now_tmp = datetime.datetime.now()
        self.str_times[(self.time_idx+1)%2] = now_tmp.strftime("%Y-%m-%d %H:%M:%S")
        self.time_idx = (self.time_idx+1)%2

    def setloger(self, loger):
        self.loger = loger

