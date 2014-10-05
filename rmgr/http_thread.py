#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

import sys
import msg
from request_handler import req_handler
import httplib

class http_thread(object):

    def __init__(self, worker):
        self.worker = worker

    def handler(self, data):
        print ('msg class: ' + str(data['class']))
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
            print ('request task response: ' + str(response.status))
            conn.close()
            return (None, response.status)
        res = response.read()
        conn.close()
        return (res, response.status)

