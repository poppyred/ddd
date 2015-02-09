#/bin/sh

lincnt=`wc -l /usr/local/dnspro/mgr.log | awk '{print $1}'`
echo $lincnt
if [ $lincnt -ge 1000000 ]
then
echo "backup"
now=`date  +%Y-%m-%d-%H-%M-%S`
cat /dev/null > /usr/local/dnspro/mgr.log
fi
