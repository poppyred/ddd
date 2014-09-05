#!/usr/bin/python
# -*- coding:UTF-8 -*-

import time
import sys, signal
import webs
import msg
import select
import termios
import tty
import mgr_conf
from mgr_singleton import g_factory
import request_handler
from mgr_misc import _lineno
import traceback
from mgr_daemon import Daemon

#values=[]
#for i in range(20):
#    values.append((i,'hi rollen'+str(i)))

#conn.query_many('insert into test values(%s,%s)',values)
#conn.commit()

def stop_all():
    if g_factory.get_check_thread().isAlive():
        g_factory.get_check_thread().stop()
        g_factory.get_check_thread().join()
    if g_factory.get_http_thread().isAlive():
        g_factory.get_http_thread().stop()
        g_factory.get_http_thread().join()
    if g_factory.get_repth_thread().isAlive():
        g_factory.get_repth_thread().stop()
        g_factory.get_repth_thread().join()
    if g_factory.get_mgr_worker4init().isAlive():
        g_factory.get_mgr_worker4init().stop()
        g_factory.get_mgr_worker4init().join()
    if g_factory.get_mgr_worker().isAlive():
        g_factory.get_mgr_worker().stop()
        g_factory.get_mgr_worker().join()
    if g_factory.get_mgr_loger().isAlive():
        g_factory.get_mgr_loger().stop()
        g_factory.get_mgr_loger().join()

def start_web():
    webs.run_websvr()

def main_loop():
    while 1:
        msg.g_now += 1
        g_factory.get_mgr_loger()._uptime()
        #g_factory.get_mgr_loger().debug(_lineno(), msg.g_now)
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
                g_factory.get_mgr_loger().error(_lineno(), 'KeyboardInterrupt:%s' % e) #可以
                g_factory.get_mgr_loger().error(traceback.format_exc())
                break
            except IOError as e:
                g_factory.get_mgr_loger().error(_lineno(), 'IOError:%s' % (e)) #可以
                g_factory.get_mgr_loger().error(traceback.format_exc())
                break
            except Exception as e:
                g_factory.get_mgr_loger().error(_lineno(), 'exception:%s' % (e,)) #可以
                g_factory.get_mgr_loger().error(traceback.format_exc())
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

        request_handler.g_req_loger = g_factory.get_mgr_loger()
        #g_factory.get_mgr_loger().debug(_lineno(), 'starting')
        #sys.exit()

        g_factory.get_repth_thread()
        g_factory.get_http_thread()
        g_factory.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_init_ok, mgr_conf.g_inner_chk_init_ok_time)
        #g_factory.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_snd, mgr_conf.g_inner_chk_snd_time)
        #g_factory.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_domain, mgr_conf.g_inner_chk_task_domain_time)
        #g_factory.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_inner_chk_task_record, mgr_conf.g_inner_chk_task_record_time)
        if False: #test
            g_factory.get_check_thread().add_tasknode_byinterval_lock(msg.g_class_init_test, mgr_conf.g_inner_chk_init_ok_time)
        g_factory.get_mgr_worker().set_buddy_thread(g_factory.get_http_thread(), g_factory.get_check_thread())

        g_factory.get_mgr_loger().start()
        time.sleep(1)
        g_factory.get_zkhandler()

        g_factory.get_mgr_worker4init().start()
        g_factory.get_mgr_worker().start()
        g_factory.get_repth_thread().start()
        time.sleep(1)
        g_factory.get_http_thread().start()
        g_factory.get_check_thread().start()

        #start_web()

        try:
            if msg.g_enable_stdin:
                msg.old_settings = termios.tcgetattr(sys.stdin)
                tty.setcbreak(sys.stdin.fileno())
        except Exception as e:
            g_factory.get_mgr_loger().error(_lineno(), 'Exception:%s' % (e,))
            g_factory.get_mgr_loger().error(traceback.format_exc())
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

