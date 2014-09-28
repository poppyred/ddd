#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import queue_thread
import sys
import msg
from request_handler import req_handler, switch
import httplib
#import urllib
from mgr_misc import _lineno

class http_thread(queue_thread.Qthread):
    http_qsize = 40000
    def __init__(self, worker, loger):
        queue_thread.Qthread.__init__(self, 'http_thread', self.http_qsize, loger)
        self.worker = worker

    def handler(self, data):
        self.loger.debug(_lineno(self), 'msg class: ', data['class'])
        for case in switch(data['class']):
            if case(msg.g_class_inner_chk_task_domain) or case(msg.g_class_inner_chk_task_record):
                req_handler.handle_inner_chk_task(self, self.worker, data['class'])
                break
            if case(msg.g_class_inner_chk_task_done):
                req_handler.handle_inner_chk_task_done(self, data)
                break
            if case():
                self.loger.warn(_lineno(self), 'recv something else: ', data['class'])

    '''
    def urldecode(self, query):
        d = {}
        a = query.split('&')
        self.loger.error(_lineno(self), repr(query))
        for s in a:
            if s.find('='):
                k,v = map(urllib.unquote, s.split('='))
                try:
                    d[k].append(v)
                except KeyError:
                    d[k] = [v]
        return d
    '''

    def http_send_post(self, _dip, _url, _body):
        headerdata = {"Host":_dip, "Content-Type":"application/x-www-form-urlencoded"}
        conn = httplib.HTTPConnection(_dip)
        conn.request(method="POST", url=_url, body=_body, headers=headerdata)
        response = conn.getresponse()
        if msg.g_http_response_OK != response.status:
            self.loger.warn(_lineno(self), 'request task response: ', response.status)
            conn.close()
            return None, response.status
        res= response.read()
        #res = urllib.unquote(res)
        conn.close()
        return res, response.status

