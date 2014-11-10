#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import threading
from mgr_misc import get_time
import types
import sys

g_err_desc = None

class ErrInfo(object):
    type_err_db = 'db'
    type_err_view = 'view'
    type_err_record = 'record'
    db_desc_lose = 'lose'
    db_desc = {db_desc_lose:'lost connection'}
    def __init__(self):
        self.desc = {self.type_err_db:[], self.type_err_view:[], self.type_err_record:[]}
        self.lock = threading.Lock()

    def add_view_timeout(self, opt, view, mask):
        self.lock.acquire()
        oldobj = self.__del_item(self.type_err_view, opt, view, mask)
        if oldobj and not self.__del_or_not4add(oldobj['sent'], oldobj['regain']):
            self.desc[self.type_err_view].append(oldobj)
        else:
            obj = {'opt':opt, 'view':view, 'mask':mask, 'settime':get_time(), 'sent':0, 'regain':0}
            self.desc[self.type_err_view].append(obj)
        #self.__travel()
        self.lock.release()

    def del_view_timeout(self, opt, view, mask):
        self.lock.acquire()
        oldobj = self.__del_item(self.type_err_view, opt, view, mask)
        if oldobj and not self.__del_or_not4del(oldobj['sent'], oldobj['regain']):
            oldobj['sent'] = 0
            oldobj['regain'] = 1
            oldobj['settime'] = get_time()
            self.desc[self.type_err_view].append(oldobj)
        #self.__travel()
        self.lock.release()

    def add_record_timeout(self, opt, view, domain, rtype):
        self.lock.acquire()
        oldobj = self.__del_item(self.type_err_record, opt, view, domain, rtype)
        if oldobj and not self.__del_or_not4add(oldobj['sent'], oldobj['regain']):
            self.desc[self.type_err_record].append(oldobj)
        else:
            obj = {'opt':opt, 'view':view, 'domain':domain, 'type':rtype, 'settime':get_time(),
                    'sent':0, 'regain':0}
            self.desc[self.type_err_record].append(obj)
        #self.__travel()
        self.lock.release()

    def del_record_timeout(self, opt, view, domain, rtype):
        self.lock.acquire()
        oldobj = self.__del_item(self.type_err_record, opt, view, domain, rtype)
        if oldobj and not self.__del_or_not4del(oldobj['sent'], oldobj['regain']):
            oldobj['sent'] = 0
            oldobj['regain'] = 1
            oldobj['settime'] = get_time()
            self.desc[self.type_err_record].append(oldobj)
        #self.__travel()
        self.lock.release()

    def add_db_error(self, desc):
        self.lock.acquire()
        if self.db_desc.has_key(desc):
            oldobj = self.__del_item(self.type_err_db, desc)
            if oldobj and not self.__del_or_not4add(oldobj['sent'], oldobj['regain']):
                self.desc[self.type_err_db].append(oldobj)
            else:
                obj = {'desc':desc, 'settime':get_time(), 'sent':0, 'regain':0}
                self.desc[self.type_err_db].append(obj)
            #self.__travel()
        self.lock.release()

    def del_db_error(self, desc):
        self.lock.acquire()
        if self.db_desc.has_key(desc):
            oldobj = self.__del_item(self.type_err_db, desc)
            if oldobj and not self.__del_or_not4del(oldobj['sent'], oldobj['regain']):
                oldobj['sent'] = 0
                oldobj['regain'] = 1
                oldobj['settime'] = get_time()
                self.desc[self.type_err_db].append(oldobj)
            #self.__travel()
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

    def __del_item(self, _type, *args):
        retobj = None
        if not self.desc.has_key(_type):
            return retobj
        arr = self.desc[_type]
        if _type == self.type_err_view:
            for i in range(len(arr)-1,-1,-1):
                if arr[i]['opt'] == args[0] \
                        and arr[i]['view'] == args[1] \
                        and arr[i]['mask'] == args[2]:
                    retobj = arr[i]
                    del arr[i]

        if _type == self.type_err_record:
            for i in range(len(arr)-1,-1,-1):
                if arr[i]['opt'] == args[0] \
                        and arr[i]['view'] == args[1] \
                        and arr[i]['domain'] == args[2] \
                        and arr[i]['type'] == args[3]:
                    retobj = arr[i]
                    del arr[i]
                    break

        if _type == self.type_err_db:
            for i in range(len(arr)-1,-1,-1):
                if arr[i]['desc'] == args[0]:
                    retobj = arr[i]
                    del arr[i]
                    break

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
        objs = [[],[]]
        objs_err = objs[0]
        objs_regain = objs[1]
        for k in self.desc:
            v = self.desc.get(k)
            if type(v) is types.ListType:
                for j in v:
                    if 1 == j['sent']:
                        continue
                    j['sent'] = 1
                    item = {'etype':k, 'desc':{}}
                    if k == self.type_err_db:
                        item['desc']['regain'] = j['regain']
                        item['desc']['settime'] = j['settime']
                        item['desc']['detail'] = self.db_desc[j['desc']]
                        item['desc']['desc'] = j['desc']

                    if k == self.type_err_view:
                        item['desc']['regain'] = j['regain']
                        item['desc']['settime'] = j['settime']
                        item['desc']['opt'] = j['opt']
                        item['desc']['view'] = j['view']
                        item['desc']['mask'] = j['mask']

                    if k == self.type_err_record:
                        item['desc']['regain'] = j['regain']
                        item['desc']['settime'] = j['settime']
                        item['desc']['opt'] = j['opt']
                        item['desc']['view'] = j['view']
                        item['desc']['domain'] = j['domain']
                        item['desc']['type'] = j['type']

                    if 1 == j['regain']:
                        objs_regain.append(item)
                    else:
                        objs_err.append(item)
        return objs

    def __travel(self):
        for k in self.desc:
            v = self.desc.get(k)
            if type(v) is types.ListType:
                print >> sys.stderr, (k + '---' + str(v))

