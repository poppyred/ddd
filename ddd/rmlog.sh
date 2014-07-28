#!/bin/bash

filepath=$(cd "$(dirname "$0")"; pwd)
logpath=$filepath/log
date=`date -d "2 days ago" +%Y-%m-%d`

rm -rf $logpath/*/$date
