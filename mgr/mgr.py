#!/usr/bin/python
# -*- coding:UTF-8 -*-
# made by likunxiang

import time
import sys, signal
import webs
import msg
import select
import termios
import tty
import mgr_conf
import mgr_singleton
from mgr_factory import g_factory
from mgr_misc import _lineno
import traceback
from mgr_daemon import Daemon
import request_handler

#values=[]
#for i in range(20):
#    values.append((i,'hi rollen'+str(i)))

#conn.query_many('insert into test values(%s,%s)',values)
#conn.commit()

def stop_all():
    if mgr_singleton.g_singleton.get_check_thread().isAlive():
        mgr_singleton.g_singleton.get_check_thread().stop()
        mgr_singleton.g_singleton.get_check_thread().join()
    if mgr_singleton.g_singleton.get_http_thread().isAlive():
        mgr_singleton.g_singleton.get_http_thread().stop()
        mgr_singleton.g_singleton.get_http_thread().join()
    if mgr_singleton.g_singleton.get_reply_thread().isAlive():
        mgr_singleton.g_singleton.get_reply_thread().stop()
        mgr_singleton.g_singleton.get_reply_thread().join()
    if mgr_singleton.g_singleton.get_worker4init().isAlive():
        mgr_singleton.g_singleton.get_worker4init().stop()
        mgr_singleton.g_singleton.get_worker4init().join()
    if mgr_singleton.g_singleton.get_worker().isAlive():
        mgr_singleton.g_singleton.get_worker().stop()
        mgr_singleton.g_singleton.get_worker().join()
    if mgr_singleton.g_singleton.get_loger().isAlive():
        mgr_singleton.g_singleton.get_loger().stop()
        mgr_singleton.g_singleton.get_loger().join()

def start_web():
    webs.run_websvr()

def main_loop():
    while 1:
        msg.g_now += 1
        mgr_singleton.g_singleton.get_loger()._uptime()
        #mgr_singleton.g_singleton.get_loger().debug(_lineno(), msg.g_now)
        if not msg.g_enable_stdin:
            time.sleep(1)
        else:
            # 获得用户输入
            try:
                if select.select([sys.stdin], [], [], 1) == ([sys.stdin], [], []):
                    line = sys.stdin.readline()
                    sys.stdout.write(line)
                    sys.stdout.flush()
                    if not line or line.rstrip()=='wwq':
                        break
            except KeyboardInterrupt as e:
                mgr_singleton.g_singleton.get_loger().error(_lineno(), 'KeyboardInterrupt:%s' % e) #可以
                mgr_singleton.g_singleton.get_loger().error(traceback.format_exc())
                break
            except IOError as e:
                mgr_singleton.g_singleton.get_loger().error(_lineno(), 'IOError:%s' % (e)) #可以
                mgr_singleton.g_singleton.get_loger().error(traceback.format_exc())
                break
            except Exception as e:
                mgr_singleton.g_singleton.get_loger().error(_lineno(), 'exception:%s' % (e,)) #可以
                mgr_singleton.g_singleton.get_loger().error(traceback.format_exc())
                break

    if msg.g_enable_stdin and msg.old_settings:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, msg.old_settings)

    stop_all()

def sigint_handler(signum,frame):
    if msg.g_enable_stdin:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, msg.old_settings)
    stop_all()
    sys.exit()


class MyDaemon(Daemon):
    def run(self):
        signal.signal(signal.SIGINT,sigint_handler)

        mgr_singleton.g_singleton = mgr_singleton.mgr_singleton(g_factory)
        #mgr_singleton.g_singleton.get_loger().debug(_lineno(), 'starting')
        #sys.exit()

        mgr_singleton.g_singleton.get_reply_thread()
        mgr_singleton.g_singleton.get_http_thread()
        if False:
            mgr_singleton.g_singleton.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_init_ok, mgr_conf.g_inner_chk_init_ok_time)
        #mgr_singleton.g_singleton.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
        #mgr_singleton.g_singleton.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_domain, mgr_conf.g_inner_chk_task_domain_time)
        #mgr_singleton.g_singleton.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_record, mgr_conf.g_inner_chk_task_record_time)
        if False: #test
            mgr_singleton.g_singleton.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_init_test, mgr_conf.g_inner_chk_init_ok_time)
        mgr_singleton.g_singleton.get_worker().set_buddy_thread(mgr_singleton.g_singleton.get_http_thread(), mgr_singleton.g_singleton.get_check_thread())

        mgr_singleton.g_singleton.get_loger().start()
        time.sleep(1)
        mgr_singleton.g_singleton.get_zkhandler()

        mgr_singleton.g_singleton.get_worker4init().start()
        mgr_singleton.g_singleton.get_worker().start()
        mgr_singleton.g_singleton.get_reply_thread().start()
        time.sleep(1)
        mgr_singleton.g_singleton.get_http_thread().start()
        mgr_singleton.g_singleton.get_check_thread().start()
        mgr_singleton.g_singleton.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_db_heartbeat,
                mgr_conf.g_inner_chk_task_db_heartbeat)

        #start_web()

        try:
            if msg.g_enable_stdin:
                msg.old_settings = termios.tcgetattr(sys.stdin)
                tty.setcbreak(sys.stdin.fileno())
        except Exception as e:
            mgr_singleton.g_singleton.get_loger().error(_lineno(), 'Exception:%s' % (e,))
            mgr_singleton.g_singleton.get_loger().error(traceback.format_exc())
        finally:
            main_loop()


if __name__ == '__main__':
    daemon = MyDaemon('/tmp/mgr.pid')
    if len(sys.argv) == 2:
        if 'start' == sys.argv[1]:
            daemon.start()
        elif 'stop' == sys.argv[1]:
            daemon.stop()
        elif 'restart' == sys.argv[1]:
            daemon.restart()
        elif 'pty' == sys.argv[1]:
            daemon.run()
        else:
            print "Unknown command"
            sys.exit(2)
        sys.exit(0)
    else:
        print "usage: %s start|stop|restart|pty" % sys.argv[0]
        sys.exit(2)

