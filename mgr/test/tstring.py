#!/usr/bin/python
# -*- coding:UTF-8 -*-

str_class = '3.16.10.10.in-addr.arpa'.partition('.in-')
print repr(str_class)

if len(str_class) < 1:
    exit

str_class = str_class[0].split(".")
split_len = len(str_class)
str_ret = "";
for i in range(split_len-1,-1,-1):
    print "%d %d\n" % (i, split_len)
    if i != 0:
        str_ret += str_class[i] + "."
    else:
        str_ret += str_class[i]
print repr(str_ret)
