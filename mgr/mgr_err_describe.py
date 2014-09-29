#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import threading
from mgr_misc import _lineno, switch

class ErrInfo(object):
    db_desc = {'lose':'lost connection'}
    def __init__(self, loger):
        self.desc = {'db':[], 'view_timeout':[], 'record_timeout':[]}
        self.lock = threading.Lock()
        self.loger = loger

    def add_view_timeout(self, opt, view, mask):
        self.lock.acquire()
        self.__del_item('view_timeout', opt, view, mask)
        obj = {'opt':opt, 'view':view, 'mask':mask, 'settime':mgr_misc.get_time()}
        self.desc['view_timeout'].append(obj)
        self.__travel()
        self.lock.release()

    def add_recored_timeout(self, opt, view, domain, rtype):
        self.lock.acquire()
        self.__del_item('record_timeout', opt, view, domain, rtype)
        obj = {'opt':opt, 'view':view, 'domain':domain, 'type':rtype, 'settime':mgr_misc.get_time()}
        self.desc['record_timeout'].append(obj)
        self.__travel()
        self.lock.release()

    def add_db_error(self, desc):
        self.lock.acquire()
        if self.db_desc.has_key(desc):
            self.__del_item('db', desc)
            obj = {'desc':desc, 'settime':mgr_misc.get_time()}
            self.desc['db'].append(obj)
            self.__travel()
        self.lock.release()

    def del_item_lock(self, _type, *args):
        self.lock.acquire()
        self.__del_item(_type, args)
        self.lock.release()

    def __del_item(self, _type, *args):
        if not self.desc.has_key(_type):
            return
        arr = self.desc[_type]
        for case in switch(_type):
            if case('view_timeout'):
                for i in range(len(arr)-1,-1,-1):
                    if arr[i]['opt'] == args[0] \
                            and arr[i]['view'] == args[1] \
                            and arr[i]['mask'] == args[2]:
                        del arr[i]
                break
            if case('record_timeout'):
                for i in range(len(arr)-1,-1,-1):
                    if arr[i]['opt'] == args[0] \
                            and arr[i]['view'] == args[1] \
                            and arr[i]['domain'] == args[2] \
                            and arr[i]['type'] == args[3]:
                        del arr[i]
                        break
                break
            if case('db'):
                for i in range(len(arr)-1,-1,-1):
                    if arr[i]['desc'] == args[0]:
                        del arr[i]
                        break
                break
            if case():
                self.loger.warn(_lineno(self), 'type %s not implemented!' % (_type,))

    def __travel(self):
        for k in self.desc:
            v = self.desc.get(k)
            if type(v) is types.ListType: #如果数据是list类型，继续遍历
                self.loger.info(_lineno(self), k, '---', v)

g_err_desc = None
