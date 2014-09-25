#!/usr/bin/python
# -*- coding:utf-8 -*-

import MySQLdb
import sys
from mgr_misc import _lineno
import traceback

__all__ = ['MySQL']

class MySQL(object):
    '''
    MySQL
    '''
    conn = None
    cursor = None
    def __init__(self,host,user,password,db,charset='utf8', loger=None):
        """MySQL Database initialization """
        self.conn_error = True
        self.host = host
        self.user = user
        self.passwd = password
        self.db = db
        self.charset = charset
        self.cursor = None
        self.loger = loger
        self.__myconnect__()

    def __myset_conn_error(self, e):
        if e.args[0] == 2006 or e.args[0] == 2003:
            self.conn_error = True

    def __myconnect__(self):
        try:
            self.loger.info(_lineno(self), 'host[%s], user[%s], passwd[%s], db[%s]' % (self.host, self.user, self.passwd, self.db))
            if self.db=='':
                if self.cursor:
                    self.cursor.close()
                if self.conn:
                    self.conn.close()
                self.conn = MySQLdb.connect(self.host,self.user,self.passwd,port=3306)
            else:
                if self.cursor:
                    self.cursor.close()
                if self.conn:
                    self.conn.close()
                self.conn = MySQLdb.connect(self.host,self.user,self.passwd,self.db,port=3306)
            self.conn_error= False
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), 'Cannot connect to server\nERROR: ', e)
            self.conn_error= True
            self.cursor = None
            self.loger.error(traceback.format_exc())
            #raise Exception("Database configure error!!!")
            return
        self.cursor = self.conn.cursor()
        self.cursor.execute("SET NAMES utf8")

    def query_many(self,sql,*values):
        """  Execute many SQL statement """
        try:
            if self.conn_error:
                self.__myconnect__()
            return self.cursor.executemany(sql,values)
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error %d: %s" % (e.args[0], e.args[1]))
            self.__myset_conn_error(e)
            self.loger.error(traceback.format_exc())
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())

    def query(self,sql,value=None):
        """  Execute SQL statement """
        try:
            if self.conn_error:
                self.__myconnect__()
            return self.cursor.execute(sql,value)
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error %d: %s" % (e.args[0], e.args[1]))
            self.__myset_conn_error(e)
            self.loger.error(traceback.format_exc())
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())

    def commit(self):
        """  commit db """
        try:
            if self.conn_error:
                self.__myconnect__()
            return self.conn.commit()
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error %d: %s" % (e.args[0], e.args[1]))
            self.__myset_conn_error(e)
            self.loger.error(traceback.format_exc())
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())

    def seldb(self,db):
        """  select db """
        try:
            if self.conn_error:
                self.__myconnect__()
            return self.conn.select_db(db)
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error %d: %s" % (e.args[0], e.args[1]))
            self.__myset_conn_error(e)
            self.loger.error(traceback.format_exc())
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())
    '''
    def call_proc_set(self):
        """ Return the results after executing SQL statement """
        try:
            if self.conn_error:
                self.__myconnect__()
            return self.cursor.stored_results()
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error: ", repr(e))
            self.__myset_conn_error(e)
            self.loger.error(traceback.format_exc())
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())
    '''

    def show(self):
        """ Return the results after executing SQL statement """
        try:
            if self.conn_error:
                self.__myconnect__()
            return self.cursor.fetchall()
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error: ", repr(e))
            self.__myset_conn_error(e)
            self.loger.error(traceback.format_exc())
            return None
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())
            return None

    def nextset(self):
        try:
            if not self.conn_error:
                self.cursor.nextset()
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error:", e)
            self.__myset_conn_error(e)
            self.loger.error(traceback.format_exc())
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())

    def fetch_proc_reset(self):
        try:
            if self.conn_error:
                self.__myconnect__()
            result = self.cursor.fetchall()
            self.cursor.close()
            self.cursor = self.conn.cursor()
            return result
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error:", e)
            self.__myset_conn_error(e)
            self.loger.error(traceback.format_exc())
            return None
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())
            return None
        return None

    def call_proc(self, proc_name, values):
        '''调用存储过程'''
        try:
            if self.conn_error:
                self.__myconnect__()
            self.cursor.callproc(proc_name, values)
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error %d: %s" % (e.args[0], e.args[1]))
            self.__myset_conn_error(e)
            self.loger.error(traceback.format_exc())
            return False
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())
            return False
        return True

    def __myclose__(self):
        """ Terminate the connection """
        try:
            if self.conn:
                self.conn.close()
            if self.cursor:
                self.cursor.close()
        except MySQLdb.Error,e:
            self.loger.error(_lineno(self), "Mysql Error %d: %s" % (e.args[0], e.args[1]))
            self.loger.error(traceback.format_exc())
        except Exception,e:
            self.loger.error(_lineno(self), "Exception: ", repr(e))
            self.loger.error(traceback.format_exc())
        finally:
            self.conn_error = True

    def __del__(self):
        """ Terminate the connection """
        self.__myclose__()

#test
if __name__ == '__main__':

    print >> sys.stderr, 'test mysql'

    mysql = MySQL(host=localhost, user='root', password='test',db='mysql')
    mysql.query('select * from users')
    result = mysql.show()
    print >> sys.stderr, len(result)
    print >> sys.stderr, result[1]

