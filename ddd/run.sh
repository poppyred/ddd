#!/bin/bash

if [ $1 = "stop" ]; then
        pid=`ps -axu |grep dnsrun.sh | grep -v grep |grep -v vim |grep -v java|grep -v gdb|grep -v log|awk '{print $2}'`
	#echo $pid
	kill -9 $pid

	
        pid=`ps -axu |grep eflydns | grep -v grep |grep -v vim |grep -v java|grep -v gdb|grep -v log|awk '{print $2}'`
	kill -9 $pid
        exit 1
elif [ $1 = "reboot" ]; then
        pid=`ps -axu |grep dnsrun.sh | grep -v grep |grep -v vim |grep -v java|grep -v gdb|grep -v log|awk '{print $2}'`
        kill -9 $pid
        pid=`ps -axu |grep eflydns | grep -v grep |grep -v vim |grep -v java|grep -v gdb|grep -v log|awk '{print $2}'`
        kill -9 $pid
	nohup sh dnsrun.sh > dnsshell.log &
	exit 1
fi  
	nohup sh dnsrun.sh > dnsshell.log &
