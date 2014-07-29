#coding:utf-8
import threading
import time
import random
import Queue
import sys

q = Queue.Queue(2)
NUM_WORKERS = 3

class MyThread(threading.Thread):
    def __init__(self,input,worktype):
        threading.Thread.__init__(self)
        self._jobq = input
        self._work_type = worktype
    def run(self):
        while True:
            print >> sys.stderr, '%d running' % (self._work_type)
            try:
                self._process_job(self._jobq.get(block=False),self._work_type)
                self._jobq.task_done()
            except Queue.Empty:
                print >> sys.stderr, '%d catch empty' % (self._work_type)
                break

    def _process_job(self, job, worktype):
        self.doJob(job,worktype)

    def doJob(self, job, worktype):
        time.sleep(random.random() * 3)
        print >> sys.stderr, "doing",job," worktype ",worktype

if __name__ == '__main__':
    print >> sys.stderr, "begin.... %s" % (q)
    try:
        for i in range(NUM_WORKERS * 2):
            q.put(i, block=False) #放入到任务队列中去
    except Queue.Full:
        print >> sys.stderr, 'queue is full, size:', q.qsize()
    finally:
        print >> sys.stderr, "job qsize:", q.qsize()
        for x in range(NUM_WORKERS):
            MyThread(q, x).start()
        q.join()
