#!/usr/bin/python
# -*- coding:UTF-8 -*-

import mgr_misc

class test():
    __slots__=('foo')
    foo=1.3;

    def misc_line(self):
        print mgr_misc._lineno(self)
    def misc_linen(self):
        print mgr_misc._lineno()

a=test()
print a.foo
a.bar=15
print a.bar
a.misc_line()
a.misc_linen()
#mgr_misc._lineno(test)

print '***********************************'



class testo(object):
    __slots__=('foo')
    foo=1.3;

a=testo()
print a.foo
a.bar=15
print a.bar
