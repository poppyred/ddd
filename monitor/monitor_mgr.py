#!/usr/bin/python
# -*- coding: utf-8 -*-  
# Filename: monitor_mgr.py

import socket
import sys,DNS
reload(sys)
sys.setdefaultencoding('utf-8')
from mail import SendEmail
from email.header import Header
import datetime
import MySQLdb
import json


addrs = []
status = {}

# 打开数据库连接
db = MySQLdb.connect("localhost","root","rjkj@rjkj","dns_device_monitor")
cursor = db.cursor()

# SQL 查询语句
sql = "SELECT * FROM server_list WHERE `type` = 'mgr'"
try:
   # 执行SQL语句
	cursor.execute(sql)
	results = cursor.fetchall()
	for row in results:
		#print row
		ip = row[0]
		type = row[1]
		statu = row[2]
		time = row[3]
		addrs.append(ip)
		status[ip] = statu
except:
   print "Error: unable to fecth data"

print addrs
print status;

query = {'class':'heartbeat'}
query = json.dumps(query)
query = str(query)

for addr in addrs:
	#HOST = '10.10.12.58'  
	PORT = 54321  
	BUFSIZE = 1024  

#timeout = 5    

	ADDR = (addr, PORT)  
	
	#print ADDR
  
	udpCliSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 
	udpCliSock.settimeout(5) 
 
	udpCliSock.sendto(query,ADDR)  
	try:
		data,ADDR = udpCliSock.recvfrom(BUFSIZE)  
		#print data  
	except:
		print "udp recive timeout"
	
	data = json.loads(data)
	print data
	
	ret = ''
	msg = ''
	
	if 'status' in data:
		ret = data['status']
		
	if 'message' in data:
		msg = data['message']

	if ret == 0:
		if status[addr] == 'false':
			
			now = datetime.datetime.now()
			now = str(now)
			now = now[:19]
			sql = "update server_list set `status` = 'true',`updatetime` = '" + now + "' where `ip` =  '" + addr + "'"

			content = "【EflyDNS监控提醒】：DNSMgr[" + addr + "]已重新恢复正常服务！"
			title = "【EflyDNS监控提醒】：DNSMgr[" + addr + "]已重新恢复正常服务！"
			#mailto_list = ['dns_monitor@efly.cc','110569339@qq.com','280057670@qq.com','13760974339@139.com','kunhho@qq.com','823969275@qq.com','56185089@qq.com','261572183@qq.com','349657492@qq.com'] 
			mailto_list = ['dns_monitor@efly.cc'] 
			mail = SendEmail('smtp.efly.cc', 'eflydns@efly.cc', 'eflydns123')
			mail.sendTxtMail(mailto_list, title , content)
			cursor.execute(sql)
		
	else:
		if status[addr] == 'true':
		
			now = datetime.datetime.now()
			now = str(now)
			now = now[:19]
			sql = "update server_list set `status` = 'false',`updatetime` = '" + now + "' where `ip` =  '" + addr + "'"

			title = "【EflyDNS监控提醒】：DNSMgr[" + addr + "]状态异常,请检查是否存在问题！"
			content = "【EflyDNS监控提醒】：DNSMgr[" + addr + "]状态异常，请检查是否存在问题！</br> 错误原因：" + str(msg) 

			#mailto_list = ['dns_monitor@efly.cc','110569339@qq.com','280057670@qq.com','13760974339@139.com','kunhho@qq.com','823969275@qq.com','56185089@qq.com','261572183@qq.com','349657492@qq.com'] 
			mailto_list = ['dns_monitor@efly.cc'] 
			mail = SendEmail('smtp.efly.cc', 'eflydns@efly.cc', 'eflydns123')
			mail.sendTxtMail(mailto_list, title , content)
			cursor.execute(sql)

			
udpCliSock.close()  

