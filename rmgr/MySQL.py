# 2014.09.28 17:31:53 CST
#Embedded file name: ./MySQL.py
import MySQLdb
import sys
from mgr_misc import _lineno
import traceback
__all__ = ['MySQL']

class MySQL(object):
    """
    MySQL
    """
    conn = None
    cursor = None

    def __init__(self, host, user, password, db, charset = 'utf8'):
        """MySQL Database initialization """
        self.conn_error = True
        self.host = host
        self.user = user
        self.passwd = password
        self.db = db
        self.charset = charset
        self.cursor = None
        self.__myconnect__()

    def __myset_conn_error(self, e):
        if e.args[0] == 2006 or e.args[0] == 2003:
            self.conn_error = True

    def __myconnect__(self):
        try:
            print 'host[%s], user[%s], passwd[%s], db[%s]' % (self.host,
             self.user,
             self.passwd,
             self.db)
            if self.db == '':
                if self.cursor:
                    self.cursor.close()
                if self.conn:
                    self.conn.close()
                self.conn = MySQLdb.connect(self.host, self.user, self.passwd, port=3306)
            else:
                if self.cursor:
                    self.cursor.close()
                if self.conn:
                    self.conn.close()
                self.conn = MySQLdb.connect(self.host, self.user, self.passwd, self.db, port=3306)
            self.conn_error = False
        except MySQLdb.Error as e:
            print ('Cannot connect to server\nERROR: ', e)
            self.conn_error = True
            self.cursor = None
            return

        self.cursor = self.conn.cursor()
        self.cursor.execute('SET NAMES utf8')

    def query_many(self, sql, *values):
        """  Execute many SQL statement """
        try:
            return self.cursor.executemany(sql, values)
        except MySQLdb.Error as e:
            self.__myset_conn_error(e)
        except Exception as e:
            pass

    def query(self, sql, value = None):
        """  Execute SQL statement """
        try:
            return self.cursor.execute(sql, value)
        except MySQLdb.Error as e:
            self.__myset_conn_error(e)
        except Exception as e:
            pass

    def commit(self):
        """  commit db """
        try:
            return self.conn.commit()
        except MySQLdb.Error as e:
            self.__myset_conn_error(e)
        except Exception as e:
            pass

    def seldb(self, db):
        """  select db """
        try:
            return self.conn.select_db(db)
        except MySQLdb.Error as e:
            self.__myset_conn_error(e)
        except Exception as e:
            pass

    def show(self):
        """ Return the results after executing SQL statement """
        try:
            return self.cursor.fetchall()
        except MySQLdb.Error as e:
            self.__myset_conn_error(e)
            return None
        except Exception as e:
            return None

    def nextset(self):
        try:
            self.cursor.nextset()
        except MySQLdb.Error as e:
            self.__myset_conn_error(e)
        except Exception as e:
            pass

    def fetch_proc_reset(self):
        try:
            result = self.cursor.fetchall()
            self.cursor.close()
            self.cursor = self.conn.cursor()
            return result
        except MySQLdb.Error as e:
            self.__myset_conn_error(e)
            return None
        except Exception as e:
            return None

    def call_proc(self, proc_name, values):
        """\xe8\xb0\x83\xe7\x94\xa8\xe5\xad\x98\xe5\x82\xa8\xe8\xbf\x87\xe7\xa8\x8b"""
        try:
            self.cursor.callproc(proc_name, values)
        except MySQLdb.Error as e:
            self.__myset_conn_error(e)
            return False
        except Exception as e:
            return False

        return True

    def __myclose__(self):
        """ Terminate the connection """
        try:
            if self.conn:
                self.conn.close()
            if self.cursor:
                self.cursor.close()
        except MySQLdb.Error as e:
            pass
        except Exception as e:
            pass
        finally:
            self.conn_error = True

    def __del__(self):
        """ Terminate the connection """
        self.__myclose__()
+++ okay decompyling MySQL.pyc 
# decompiled 1 files: 1 okay, 0 failed, 0 verify failed
# 2014.09.28 17:31:53 CST
