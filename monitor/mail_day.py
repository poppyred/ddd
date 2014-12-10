#!/usr/bin/python
# -*- coding: utf-8 -*-  
# Filename: send_email.py

from mail import SendEmail
import datetime
from pprint import pprint
import requests
from xml.etree import ElementTree as ET 

import xml.sax 
import xml.sax.handler 
 
class XMLHandler(xml.sax.handler.ContentHandler): 
    def __init__(self): 
        self.buffer = ""                   
        self.mapping = {}                 
 
    def startElement(self, name, attributes): 
        self.buffer = ""                   
 
    def characters(self, data): 
        self.buffer += data                     
 
    def endElement(self, name): 
        self.mapping[name] = self.buffer          
 
    def getDict(self): 
        return self.mapping 


now = datetime.datetime.now()
date = now.strftime('%a %B %d %Y')
hour = now.strftime('%H')
if hour != '15':
	print "时间不对"
	exit();

r = requests.get('http://xml.weather.yahoo.com/forecastrss?w=35567&u=c')


xh = XMLHandler() 
xml.sax.parseString(r.content, xh) 
ret = xh.getDict() 
 
#pprint(ret) 
string = ret['description']
string = string.split('\n')
string = string[3]
string = string[0:-8]
string = string.split(', ')

weather = string[0]
tp = string[1]


title = '肥猪心，早晨，今日圣安天气:' + str(weather) + '，当前体感温度:' + str(tp) + "℃，气温渐冷，注意保暖"
content = "<h1>早晨啦，亲!</h1><br><h1>今天天气：" + str(weather) + "，当前体感温度：" + str(tp) + "℃</h1><br><h1>正式上课了！不要再赖床了！记得忙碌之余多活动活动，多喝水，别再熬夜了</h1><br><h1>注意身体，有空的话多留点时间跟我Facetime啦!</h1><br><h1>爱你!</h1>"

mailto_list = ['460843287@qq.com'] 
#mailto_list = ['110569339@qq.com'] 
mail = SendEmail('smtp.163.com', 'chinabobby@163.com', 'xinyi520')
if mail.sendImageMail(mailto_list, title , content):  
    print "发送成功"  
else:  
    print "发送失败"
	
backup_list = ['110569339@qq.com'] 
mail = SendEmail('smtp.163.com', 'chinabobby@163.com', 'xinyi520')
if mail.sendImageMail(backup_list, title , content):  
    print "发送成功"  
else:  
    print "发送失败"
