#!/bin/bash
 
function display_usage()
{
   echo "Usage: $0 [install src | uninstall version]" >&2
   echo "Usage: $0 [install src | uninstall version]" >> /root/kernel_cmp.log
}
 
rm /root/kernel_cmp.log -f

if [ $# -ne 2 ]; then
   display_usage
   exit 1
fi
 
if [ $1 = "install" ]; then
   cd $2
   if [ $? -ne 0 ]; then
      echo "cannot enter $2 error!"
      echo "cannot enter $2 error!" >> /root/kernel_cmp.log
      exit 1
   fi
 
   if ! [ -f Makefile ]; then
      echo "cannot get kernel version."
      echo "cannot get kernel version." >> /root/kernel_cmp.log
      exit 1
   fi
 
   KERNEL_VERSION=`sed -n '1'p Makefile | awk '{print$3}'`.`sed -n '2'p Makefile | awk '{print$3}'`.`sed -n '3'p Makefile | awk '{print$3}'``sed -n '4'p Makefile | awk '{print$3}'`
   let nr_thread=`cat /proc/cpuinfo | grep processor | wc -l`*2

   #echo $KERNEL_VERSION
   #exit 1

   #make menuconfig
   #if [ $? -ne 0 ]; then
   #   exit 1
   #fi
 
   echo "-------------------- make begin --------------------"
   echo "-------------------- make begin --------------------" >> /root/kernel_cmp.log
 
   make -j $nr_thread
   if [ $? -ne 0 ]; then
   	echo "make -j $nr_thread error"
   	echo "make -j $nr_thread error" >> /root/kernel_cmp.log
	exit 1
   fi
 
   #make -j $nr_thread modules
   #if [ $? -ne 0 ]; then
   #   echo "make -j $nr_thread modules error"
   #   echo "make -j $nr_thread modules error" >> /root/kernel_cmp.log
   #   exit 1
   #fi
 
   echo "--------------------- make end ---------------------"
   echo "--------------------- make end ---------------------" >> /root/kernel_cmp.log
   echo "-------------------- install begin --------------------"
   echo "-------------------- install begin --------------------" >> /root/kernel_cmp.log
 
   make modules_install
   if [ $? -ne 0 ]; then
      echo "make modules_install error"
      echo "make modules_install error" >> /root/kernel_cmp.log
      exit 1
   fi
 
   make install
   if [ $? -ne 0 ]; then
      echo "make install error"
      echo "make install error" >> /root/kernel_cmp.log
      exit 1
   fi
 
   echo "--------------------- install end ---------------------"
   echo "--------------------- install end ---------------------" >> /root/kernel_cmp.log
   echo "-------------------- update begin --------------------"
   echo "-------------------- update begin --------------------" >> /root/kernel_cmp.log
 
   cd /boot
   KIDX=`cat grub/grub.conf | grep title | sed -n "/$KERNEL_VERSION)$/="`
   echo $KIDX
   echo $KIDX >> /root/kernel_cmp.log
   if [ ! -n "$KIDX" ]
   then
   	echo "title not found"
   	echo "title not found" >> /root/kernel_cmp.log
	exit 1
   fi

   INT_KIDX=`expr $KIDX - 1`
   echo $INT_KIDX
   echo $INT_KIDX >> /root/kernel_cmp.log
   
   DEF_IDX=`sed -n '/default=/=' grub/grub.conf`
   echo $DEF_IDX
   echo $DEF_IDX >> /root/kernel_cmp.log

   sed -i "${DEF_IDX}s/.*/default\=${INT_KIDX}/" grub/grub.conf

   #mkinitrd --force /boot/initramfs-$KERNEL_VERSION.img $KERNEL_VERSION
   #if [ $? -ne 0 ]; then
   #   exit 1
   #fi
 
   #matchline=(`sed -n '/menuentry.*'$KERNEL_VERSION'.*/'= /boot/grub/grub.cfg`)
   #let nr_line=${#matchline[@]}
   #if [ $nr_line -eq 0 ]; then
   #   echo "cannot find kernel version: $KERNEL_VERSION." >&2
   #   exit 1
   #fi
 
   #awk 'BEGIN{i = 0} {if ($0 ~ /^}/) {if (i < '$nr_line' && NR > '${matchline['i']}') {printf "\tinitrd\t%s\n}\n", "/boot/initrd.img-'$KERNEL_VERSION'"; ++i} else print $0} else print $0}' /boot/grub/grub.cfg > /tmp/tmpgrub.cfg
   #mv /tmp/tmpgrub.cfg /boot/grub/grub.cfg
 
   #update-grub
 
   echo "--------------------- update end ---------------------"
   echo "--------------------- update end ---------------------" >> /root/kernel_cmp.log
   exit 0

elif [ $1 = "uninstall" ]; then
   rm -rf /lib/modules/$2
   rm /boot/*$2*
 
   #update-grub
else
   display_usage
fi
