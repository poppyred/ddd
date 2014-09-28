#!/usr/bin/env python
# -*- coding:UTF-8 -*-
# made by likunxiang

from os.path import basename, join
from collections import namedtuple
import zookeeper, time
import mgr_conf
from mgr_misc import _lineno

ZOO_OPEN_ACL_UNSAFE = {"perms":0x1f, "scheme":"world", "id" :"anyone"}

# Mapping of connection state values to human strings.
STATE_NAME_MAPPING = {
    zookeeper.ASSOCIATING_STATE: "associating",
    zookeeper.AUTH_FAILED_STATE: "auth-failed",
    zookeeper.CONNECTED_STATE: "connected",
    zookeeper.CONNECTING_STATE: "connecting",
    zookeeper.EXPIRED_SESSION_STATE: "expired",
}

# Mapping of event type to human string.
TYPE_NAME_MAPPING = {
    zookeeper.NOTWATCHING_EVENT: "not-watching",
    zookeeper.SESSION_EVENT: "session",
    zookeeper.CREATED_EVENT: "created",
    zookeeper.DELETED_EVENT: "deleted",
    zookeeper.CHANGED_EVENT: "changed",
    zookeeper.CHILD_EVENT: "child",
}

def watchmethod(func):
    def decorated(handle, atype, state, path):
        event = ClientEvent(atype, state, path)
        return func(event)
    return decorated

class ClientEvent(namedtuple("ClientEvent", 'type, connection_state, path')):
    """
    A client event is returned when a watch deferred fires. It denotes
    some event on the zookeeper client that the watch was requested on.
    """

    @property
    def type_name(self):
        return TYPE_NAME_MAPPING[self.type]

    @property
    def state_name(self):
        return STATE_NAME_MAPPING[self.connection_state]

    def __repr__(self):
        return  "<ClientEvent %s at %r state: %s>" % (
            self.type_name, self.path, self.state_name)

class LKXZookeeper(object):
    ROOT = "/app"
    WORKERS_PATH = join(ROOT, "workers")
    MASTERS_NUM = 1
    TIMEOUT = 10000

    def __init__(self, loger):
        self.loger = loger
        self.path = None
        self.handle = -1
        self.handle = zookeeper.init(mgr_conf.g_zkClis, self.connection_watcher, self.TIMEOUT)
        self.loger.info(_lineno(), "connecting to zks[%s] timeout is[%d]" % (mgr_conf.g_zkClis, self.TIMEOUT))

    def connection_watcher(self, h, type, state, path):
        if state == zookeeper.CONNECTED_STATE:
            self.handle = h
            self.connected = True
            self.loger.info(_lineno(), "connect to zks[%s] successfully!" % mgr_conf.g_zkClis)
            self.__init_zk()
            self.register()
        else:
            self.connected = False
            self.handle = -1
            self.handle = zookeeper.init(mgr_conf.g_zkClis, self.connection_watcher, self.TIMEOUT)
            self.loger.error(_lineno(), "try to connect to zks[%s] timeout is[%d]" % (mgr_conf.g_zkClis, self.TIMEOUT))

    def __init_zk(self):
        nodes = (self.ROOT, self.WORKERS_PATH)
        for node in nodes:
            if not self.exists(node):
                try:
                    self.create(node, "")
                except:
                    pass

    def register(self):
        self.path = self.WORKERS_PATH + "/" + mgr_conf.g_mgr_sid
        if not self.exists(self.path):
            self.path = self.create(self.path, self.loger.get_level(), flags=zookeeper.EPHEMERAL )
        self.path = basename(self.path)
        self.loger.info(_lineno(), "register ok! I'm %s" % self.path)
        # check who is the master
        self.get_level()

    def get_level(self):
        @watchmethod
        def level_watcher(event):
            if event.connection_state != zookeeper.CONNECTED_STATE:
                self.connected = False
                self.handle = -1
                self.handle = zookeeper.init(mgr_conf.g_zkClis, self.connection_watcher, self.TIMEOUT)
                self.loger.error(_lineno(), "reconnecting to zks[%s] timeout is[%d]" % (mgr_conf.g_zkClis, self.TIMEOUT))
                return
            value = self.get(event.path, level_watcher)
            self.loger.info(_lineno(), "log level change %s:[%s]" % (event.path, value[0]))
            self.loger.set_level(value[0])

        value = self.get(self.WORKERS_PATH + '/' + self.path, level_watcher)
        self.loger.info(_lineno(), "init log level %s:[%s]" % (self.WORKERS_PATH + '/' + self.path, value[0]))
        self.loger.set_level(value[0])

    def create(self, path, data="", flags=0, acl=[ZOO_OPEN_ACL_UNSAFE]):
        start = time.time()
        result = zookeeper.create(self.handle, path, data, acl, flags)
        self.loger.info(_lineno(), "Node %s created in %d ms" % (path, int((time.time() - start) * 1000)))
        return result

    def exists(self, path, watcher=None):
        return zookeeper.exists(self.handle, path, watcher)

    def get(self, path, watcher=None):
        return zookeeper.get(self.handle, path, watcher)


def main():
    lkx_zookeeper = LKXZookeeper()

if __name__ == "__main__":
    main()
    import time
    time.sleep(1000)
