#!/bin/bash
# made by likunxiang

ps -ef | grep python | grep mgr.py | grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | grep python | grep mgr.py | grep -v grep
