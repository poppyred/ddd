#!/bin/bash

pid=`ps axu |grep eflydns | grep -v grep |grep -v vim |grep -v java|grep -v gdb|grep -v log| wc -l`
if [ "$pid" -gt 0 ]
then
pid=`ps axu |grep eflydns | grep -v grep |grep -v vim |grep -v java|grep -v gdb|grep -v log| awk '{print $11}'`
echo "[$pid] is already running..."
echo "please kill it before run this shell."
echo "dnsrun.sh will exit."
exit
fi

filepath=$(cd "$(dirname "$0")"; pwd)
if [ $# -eq 1 ]
then
filename=$1
else
filename=eflydns
fi
file=$filepath/$filename
exedir=$filepath/exe
date=`date +%Y-%m-%d`

if [ -f $file ]
then
cp -f $file ${file}_$date
else
echo "file[$file] does not exist."
exit
fi

filename=${filename}_$date
file=${file}_$date
echo $file

while [ 1 ]; do
pid=`ps axu |grep $filename | grep -v grep |grep -v vim |grep -v java|grep -v gdb|grep -v log| wc -l`
if [ "$pid" -gt 0 ]
then
echo "$file is still alive."
else
sleep 1
nohup $file > /var/log/eflydns.log 2>&1 &
mkdir $exedir
eflydnspid=`ps axu |grep $filename | grep -v grep |grep -v vim |grep -v java|grep -v gdb|grep -v log|awk '{print $2}'`
cp -f $file $exedir/$filename.$eflydnspid
fi
sleep 1
done
