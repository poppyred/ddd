#!/usr/bin/python
# -*- coding: utf-8 -*- 
# Filename: send_email.py


import sys
reload(sys)
sys.setdefaultencoding('utf-8')
from mail import SendEmail


content = "这是一封来自DNS监控的测试邮件，请勿回复！"
title = "这是一封来自DNS监控的测试邮件，请勿回复！"
	
mailto_list = ['dns_monitor@efly.cc','110569339@qq.com','280057670@qq.com','495903755@qq.com','kunhho@qq.com','823969275@qq.com','56185089@qq.com','261572183@qq.com','349657492@qq.com'] 
mail = SendEmail('smtp.efly.cc', 'eflydns@efly.cc', 'eflydns123')
mail.sendTxtMail(mailto_list, title , content)