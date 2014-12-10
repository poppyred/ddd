#!/usr/bin/python
# -*- coding: utf-8 -*- 
# Filename: monitor_core.py
	
import sys,DNS
reload(sys)
sys.setdefaultencoding('utf-8')
from mail import SendEmail
from email.header import Header
import datetime

import MySQLdb

addrs = []
status = {}

# 打开数据库连接
db = MySQLdb.connect("localhost","root","rjkj@rjkj","dns_device_monitor")
cursor = db.cursor()

# SQL 查询语句
sql = "SELECT * FROM server_list WHERE `type` = 'core'"
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

Domain = 'www.test.com'

for addr in addrs:

	try:
		s = DNS.Request(name=Domain, server=addr,timeout=10)
		records = s.req().answers 
		if not len(records):
			print "Not found"
		for i in records:
			print addr + ":[%s - %s]"%(i['typename'],i['data'])
			
	
		if status[addr] == 'false':
			now = datetime.datetime.now()
			now = str(now)
			now = now[:19]
			sql = "update server_list set `status` = 'true',`updatetime` = '" + now + "' where `ip` =  '" + addr + "'"

			content = "【EflyDNS监控提醒】：DNSCore[" + addr + "]已重新恢复正常服务！"
			title = "【EflyDNS监控提醒】：DNSCore[" + addr + "]已重新恢复正常服务！"
			#mailto_list = ['huangyb@efly.cc'] 
			mailto_list = ['dns_monitor@efly.cc','110569339@qq.com','280057670@qq.com','13760974339@139.com','kunhho@qq.com','823969275@qq.com','56185089@qq.com','261572183@qq.com','349657492@qq.com'] 
			mail = SendEmail('smtp.efly.cc', 'eflydns@efly.cc', 'eflydns123')
			mail.sendTxtMail(mailto_list, title , content)
			cursor.execute(sql)


	except:
	
		if status[addr] == 'true':
			now = datetime.datetime.now()
			now = str(now)
			now = now[:19]
			sql = "update server_list set `status` = 'false',`updatetime` = '" + now + "' where `ip` =  '" + addr + "'"

			content = "【EflyDNS监控提醒】：DNSCore[" + addr + "]失去响应,请检查是否存在问题！"
			title = "【EflyDNS监控提醒】：DNSCore[" + addr + "]失去响应，请检查是否存在问题！"
			mailto_list = ['dns_monitor@efly.cc','110569339@qq.com','280057670@qq.com','13760974339@139.com','kunhho@qq.com','823969275@qq.com','56185089@qq.com','261572183@qq.com','349657492@qq.com'] 
			#mailto_list = ['huangyb@efly.cc','maij@efly.cc','shiw@efly.cc','likx@efly.cc','chenjh@efly.cc','zengxl@efly.cc'] 
			mail = SendEmail('smtp.efly.cc', 'eflydns@efly.cc', 'eflydns123')
			mail.sendTxtMail(mailto_list, title , content)
			cursor.execute(sql)
     
	 
# 关闭数据库连接
db.close()