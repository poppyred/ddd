#!/usr/bin/env python
# -*- coding:UTF-8 -*-

from SocketServer import TCPServer,StreamRequestHandler
class Handler(StreamRequestHandler):
	def handle(self):
		addr = self.request.getpeername()
		print 'Got connection from ',addr
		while True:
	        	data = self.request.recv(1024)
	 		print len(data)
	 		if len(data) <= 0:
                        	break

server = TCPServer(('',12345), Handler)	#实例化服务类对象

server.serve_forever()	#开启服务

