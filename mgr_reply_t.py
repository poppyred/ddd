#!/usr/bin/env python

from socket import *

HOST = '192.168.85.165'
PORT = 54321
BUFSIZE = 1024

ADDR = (HOST, PORT)

udpCliSock = socket(AF_INET, SOCK_DGRAM)

while True:
    data = raw_input('>')
    if not data:
        break
    udpCliSock.sendto(data,ADDR)

udpCliSock.close()

