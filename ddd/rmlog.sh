#!/bin/bash

filepath=$(cd "$(dirname "$0")"; pwd)
logpath=$filepath/log
date=`date -d "2 days ago" +%Y-%m-%d`

#echo $date
rm -rf $logpath/*/$date
