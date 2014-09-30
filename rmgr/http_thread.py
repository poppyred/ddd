#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import queue_thread
import sys
import msg
from request_handler import req_handler
import httplib

class http_thread(queue_thread.Qthread):
    http_qsize = 40000

    def __init__(self, worker):
        queue_thread.Qthread.__init__(self, 'http_thread', self.http_qsize)
        self.worker = worker

    def handler(self, data):
        print ('msg class: ', data['class'])
        if data['class'] == msg.g_class_inner_chk_task_domain or data['class'] == msg.g_class_inner_chk_task_record:
            req_handler.handle_inner_chk_task(self, self.worker, data['class'])
        if data['class'] == msg.g_class_inner_chk_task_done:
            req_handler.handle_inner_chk_task_done(self, data)

    def http_send_post(self, _dip, _url, _body):
        headerdata = {'Host': _dip,
         'Content-Type': 'application/x-www-form-urlencoded'}
        conn = httplib.HTTPConnection(_dip)
        conn.request(method='POST', url=_url, body=_body, headers=headerdata)
        response = conn.getresponse()
        if msg.g_http_response_OK != response.status:
            print ('request task response: ', response.status)
            conn.close()
            return (None, response.status)
        res = response.read()
        conn.close()
        return (res, response.status)
