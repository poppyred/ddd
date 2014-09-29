#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import threading
from mgr_misc import _lineno, switch, get_time
import types

g_err_desc = None

class ErrInfo(object):
    desc_lose = 'lose'
    db_desc = {desc_lose:'lost connection'}
    def __init__(self, loger):
        self.desc = {'db':[], 'view':[], 'record':[]}
        self.lock = threading.Lock()
        self.loger = loger

    def add_view_timeout(self, opt, view, mask):
        self.lock.acquire()
        oldobj = self.__del_item('view', opt, view, mask)
        if oldobj and not self.__del_or_not4add(oldobj['sent'], oldobj['regain']):
            self.desc['view'].append(oldobj)
        else:
            obj = {'opt':opt, 'view':view, 'mask':mask, 'settime':get_time(), 'sent':0, 'regain':0}
            self.desc['view'].append(obj)
        self.__travel()
        self.lock.release()

    def add_recored_timeout(self, opt, view, domain, rtype):
        self.lock.acquire()
        oldobj = self.__del_item('record', opt, view, domain, rtype)
        if oldobj and not self.__del_or_not4add(oldobj['sent'], oldobj['regain']):
            self.desc['record'].append(oldobj)
        else:
            obj = {'opt':opt, 'view':view, 'domain':domain, 'type':rtype, 'settime':get_time(),
                    'sent':0, 'regain':0}
            self.desc['record'].append(obj)
        self.__travel()
        self.lock.release()

    def add_db_error(self, desc):
        self.lock.acquire()
        if self.db_desc.has_key(desc):
            oldobj = self.__del_item('db', desc)
            if oldobj and not self.__del_or_not4add(oldobj['sent'], oldobj['regain']):
                self.desc['db'].append(oldobj)
            else:
                obj = {'desc':desc, 'settime':get_time(), 'sent':0, 'regain':0}
                self.desc['db'].append(obj)
            self.__travel()
        self.lock.release()

    def del_db_error(self, desc):
        self.lock.acquire()
        if self.db_desc.has_key(desc):
            oldobj = self.__del_item('db', desc)
            if oldobj and not __del_or_not4del(oldobj['sent'], oldobj['regain']):
                oldobj['sent'] = 0
                oldobj['regain'] = 1
                oldobj['settime'] = get_time()
                self.desc['db'].append(oldobj)
            self.__travel()
        self.lock.release()

    def del_item_lock(self, _type, *args):
        self.lock.acquire()
        self.__del_item(_type, args)
        self.lock.release()

    def __del_or_not4del(self, sent, regain):
        if sent == 0 and regain == 0 or sent == 1 and regain == 1:
            return True
        return False

    def __del_or_not4add(self, sent, regain):
        if sent == 0 and regain == 0 or sent == 1 and regain == 0:
            return False
        return True

    def __del_item(self, clean, _type, *args):
        retobj = None
        if not self.desc.has_key(_type):
            return retobj
        arr = self.desc[_type]
        for case in switch(_type):
            if case('view'):
                for i in range(len(arr)-1,-1,-1):
                    if arr[i]['opt'] == args[0] \
                            and arr[i]['view'] == args[1] \
                            and arr[i]['mask'] == args[2]:
                        retobj = arr[i]
                        del arr[i]
                break
            if case('record'):
                for i in range(len(arr)-1,-1,-1):
                    if arr[i]['opt'] == args[0] \
                            and arr[i]['view'] == args[1] \
                            and arr[i]['domain'] == args[2] \
                            and arr[i]['type'] == args[3]:
                        retobj = arr[i]
                        del arr[i]
                        break
                break
            if case('db'):
                for i in range(len(arr)-1,-1,-1):
                    if arr[i]['desc'] == args[0]:
                        retobj = arr[i]
                        del arr[i]
                        break
                break
            if case():
                self.loger.warn(_lineno(self), 'type %s not implemented!' % (_type,))

        return retobj

    def gen_msg(self):
        self.lock.acquire()
        ret_msg = self.__to_array()
        for k in self.desc:
            v = self.desc.get(k)
            if type(v) is types.ListType:
                for i in range(len(v)-1,-1,-1):
                    if v[i]['sent'] == 1 and v[i]['regain'] == 1:
                        del v[i]
        self.lock.release()
        return ret_msg

    def __to_array(self):
        objs = []
        for k in self.desc:
            v = self.desc.get(k)
            if type(v) is types.ListType: #如果数据是list类型，继续遍历
                for j in v:
                    if j['sent'] == 1:
                        continue
                    j['sent'] = 1
                    if k == 'db':
                        j['detail'] = self.db_desc[j['desc']]
                    objs.append({'etype':k, 'desc':j})
        return objs

    def __travel(self):
        for k in self.desc:
            v = self.desc.get(k)
            if type(v) is types.ListType: #如果数据是list类型，继续遍历
                self.loger.info(_lineno(self), k, '---', v)

