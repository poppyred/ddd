#!/bin/bash
if [ $# -lt 1 ] ; then
    echo 'arguments lack'
    exit 1
fi


exp_lev=$(echo $1 | tr 'A-Z' 'z-z')
if [ "$exp_lev" = "debug" ] || [ "$exp_lev" = "info" ] || \
	   [ "$exp_lev" = "warn" ] || [ "$exp_lev" = "error" ] ; then 
    sed -i "/log_level/c log_level=\'$exp_lev\'" $MGRHOME/mgr_conf.py
    echo "success!!" 
else 
    echo "level error!!" 
fi
