#!/bin/bash

python $MGRHOME/mgr.pyc stop
ps -ef | grep python | grep mgr.pyc | grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | grep python | grep mgr.pyc | grep -v grep
