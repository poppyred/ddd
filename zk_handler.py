#!/usr/bin/env python
# -*- coding:UTF-8 -*-

from os.path import basename, join
from zkclient import ZKClient, zookeeper, watchmethod
import mgr_conf
from mgr_misc import _lineno

class GJZookeeper(object):

    ROOT = "/app"
    WORKERS_PATH = join(ROOT, "workers")
    MASTERS_NUM = 1
    TIMEOUT = 10000

    def __init__(self, loger):
        self.path = None
        self.loger = loger

        self.zk = ZKClient(mgr_conf.g_zkClis, timeout = self.TIMEOUT)
        self.loger.info(_lineno(), "login ok!")
        # init
        self.__init_zk()
        # register
        self.register()

    def __init_zk(self):
        """
        create the zookeeper node if not exist
        """
        nodes = (self.ROOT, self.WORKERS_PATH)
        for node in nodes:
            if not self.zk.exists(node):
                try:
                    self.zk.create(node, "")
                except:
                    pass

    def register(self):
        self.path = self.zk.create(self.WORKERS_PATH + "/" + mgr_conf.g_mgr_sid, "info", flags=zookeeper.EPHEMERAL )
        self.path = basename(self.path)
        self.loger.info(_lineno(), "register ok! I'm %s" % self.path)
        # check who is the master
        self.get_level()

    def get_level(self):
        @watchmethod
        def level_watcher(event):
            #value = self.zk.get(self.WORKERS_PATH + '/' + self.path, level_watcher)
            value = self.zk.get(event.path, level_watcher)
            #self.loger.info(_lineno(), "log level change %s:[%s]" % (self.WORKERS_PATH + '/' + self.path, value[0]))
            self.loger.info(_lineno(), "log level change %s:[%s]" % (event.path, value[0]))
            self.loger.set_level(value[0])

        value = self.zk.get(self.WORKERS_PATH + '/' + self.path, level_watcher)
        self.loger.info(_lineno(), "init log level %s:[%s]" % (self.WORKERS_PATH + '/' + self.path, value[0]))
        self.loger.set_level(value[0])


def main():
    gj_zookeeper = GJZookeeper()

if __name__ == "__main__":
    main()
    import time
    time.sleep(1000)
