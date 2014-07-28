#!/bin/bash
#gateway=120.31.132.1
#route add -host 10.11.255.22 gw $gateway
pid=`ps axu |grep eflydns | grep -v grep |grep -v vim|grep -v sh | wc -l`
if [ "$pid" -gt 0 ]
then
echo "eflydns is running."
exit 0
fi
cd /root/NewDns
nohup /root/NewDns/eflydns > /var/log/eflydns.log 2>&1 &
