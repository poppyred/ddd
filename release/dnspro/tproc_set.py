#!/usr/bin/python
# -*- coding:UTF-8 -*-

import MySQLdb
cnx = MySQLdb.connect('121.201.11.3', 'root', 'qwer','dnspro_core')
cursor = cnx.cursor()

for i in range(0,2):
    print i
    try:
        cursor.callproc("del_a_domain", ('test3.com',))
        ars=cursor.fetchall()
        while ars and len(ars)>0:
            print ars
            cursor.nextset()
            ars=cursor.fetchall()
        cursor.close()
        cursor = cnx.cursor()

        print '***********************'

        cursor.callproc("del_a_domain", ('test2.com',))
        ars=cursor.fetchall()
        while ars and len(ars)>0:
            print ars
            cursor.nextset()
            ars=cursor.fetchall()
    except Exception as e:
        print(e)
