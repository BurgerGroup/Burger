#!/usr/bin/python
# -*- coding: utf-8 -*-
import socket, sys

# BlockingIOError: [Errno 11] Resource temporarily unavailable
# python3 tcpConnection_test03.py 10000000 100
# 如果第一次write没有能够发送完数据，第二次调用write几乎肯定返回EAGAIN

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

sock.connect(('123.56.48.126', 8888))  # 最好连接到网络上的一台机器

sock.setblocking(0)

a = 'a' * int(sys.argv[1])  # 两条消息由命令行给出，a应该足够大
b = 'b' * int(sys.argv[2])  

n1 = sock.send(a.encode())  # 第一次发送
n2 = 0
try:
    n2 = sock.send(b.encode())  # 第二次发送，遇到EAGAIN会抛socket.error异常
except socket.error as ex:
    print(ex)

print(n1)
print(n2)
sock.close()