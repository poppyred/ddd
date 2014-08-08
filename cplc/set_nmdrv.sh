#!/bin/bash
DRV_DIR=/opt/ixgbe_netmap_drv

rm /root/set_nmdrv.log -f

echo "building driver" >> /root/set_nmdrv.log
echo "building driver"

cd $DRV_DIR/LINUX

make clean && make
if [ $? -ne 0 ]; then
	echo "build drive error" >> /root/set_nmdrv.log
	echo "build drive error"
	exit 1
fi

sed -i "/grep proxy/"d /etc/init.d/network
sed -i "/^  stop/a dnspid=\`ps axu|grep proxy|grep -v grep|grep -v vim|grep -v java|grep -v gdb|grep -v log|wc -l\`;if [ \"\$dnspid\" -gt 0 ];then ps -ef|grep proxy|grep -v grep|awk '{print \$2}'|xargs kill -9;ps -ef|grep dnsrun|grep -v grep|awk '{print \$2}'|xargs kill -9;fi" /etc/init.d/network
/bin/cp /lib/modules/$(uname -r)/kernel/drivers/net/ixgbe/ixgbe.ko /lib/modules/$(uname -r)/kernel/drivers/net/ixgbe/ixgbe.ko_bak -f
/bin/cp $DIR/$DRV_DIR/LINUX/ixgbe/ixgbe.ko /lib/modules/$(uname -r)/kernel/drivers/net/ixgbe/ -f
/bin/cp $DIR/$DRV_DIR/LINUX/netmap_lin.ko /lib/modules/$(uname -r)/kernel/drivers/net/ixgbe/ -f
depmod -eF /boot/System.map-`uname -r` -A
if [ $? -ne 0 ]; then
    echo "update ixgbe driver fail!" >> /root/set_nmdrv.log
    echo "update ixgbe driver fail!"
    exit 1
fi

sed -i "/setenforce/"d /etc/rc.d/rc.local
echo "/usr/sbin/setenforce 0" >> /etc/rc.d/rc.local

echo "update ixgbe driver success!" >> /root/set_nmdrv.log
echo "update ixgbe driver success!"
