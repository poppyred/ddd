#!/bin/bash

# Function : transfer directory from localhost to remote node
# args       : ip password local_dir remote_dir
# Author   : firefoxbug
# Date     : 2013/06/25

if [ $# -ne 0 ]
then
    echo "[ERROR] usage : ./send_dir ip source_dir dst_dir"
    exit 1
fi
ip='121.201.12.57'
#ip=$1
port="22"
passwd='rjkj@dnspro#0801'
local_dir='release/dnspro'
remote_dir='/usr/local'

#/bin/rpm -qa|/bin/grep -q expect
#if [ $? -ne 0 ];then
#    echo "please install expect"
#    exit
#fi

#/bin/mv dnsprosql.sql dnspro/ soaplib-master.zip cscope.* mk_tags tags ~ -f

oldline=`sed -n '/g_enable_stdin/p' msg.py`
sed -i '/g_enable_stdin/c g_enable_stdin=0' msg.py

oldloglevel=`sed -n '/log_level/p' mgr_conf.py`
sed -i "/log_level/c log_level=\'info\'" mgr_conf.py

oldlogdir=`sed -n '/log_direction/p' mgr_conf.py`
sed -i "/log_direction/c log_direction=1" mgr_conf.py

python -c "import compileall; compileall.compile_dir('.')"
mkdir -p $local_dir
/bin/rm $local_dir/* -rf
/bin/mv *.pyc $local_dir
#/bin/cp mgr_conf.py chlog.sh init.sh stop.sh start.sh psmgr.sh $local_dir -rf
/bin/cp *.py chlog.sh init.sh stop.sh start.sh psmgr.sh $local_dir -rf

expect -c " 
        spawn scp -r -P $port $local_dir root@$ip:$remote_dir
        expect {
            \"*assword\" {set timeout 60000; send \"$passwd\r\";}
            \"yes/no\" {send \"yes\r\"; exp_continue;}
        }
        expect eof"

expect -c "
        spawn ssh root@$ip
        expect {
            \"*yes/no\" { send \"yes\r\"; exp_continue;}
            \"*assword:\" {set timeout 60000; send \"$passwd\r\";}
        }
        expect \"#*\"
            send \"cd $remote_dir/dnspro\r\"
            send \"source init.sh\r\"
            send \"exit\r\"
        expect eof"

sed -i "/g_enable_stdin/c $oldline" msg.py
sed -i "/log_level/c $oldloglevel" mgr_conf.py
sed -i "/log_direction/c $oldlogdir" mgr_conf.py
/bin/rm $local_dir/* -rf
