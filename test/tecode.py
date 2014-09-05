#!/usr/bin/python
# -*- coding:UTF-8 -*-

import urllib

print urllib.quote('中国')
print urllib.unquote('data=%E4%B8%AD%E5%9B%BD') #.decode("utf-8").encode("gb2312")
print repr(u'中文')
print u'中文'
print u'\u4e2d\u6587'
print '\u4e2d\u6587'
print repr('中文'.decode('utf8'))
print '\xe4\xb8\xad\xe6\x96\x87'

g_d = {'v1':1, 'v2':2}
g_v2 = g_d.pop('v2')
print repr(g_v2)
#g_v3 = g_d.pop('v3') #抛异常为！！
#print repr(g_v3)

for i in range(2):
    print i
print i
