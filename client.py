# -*- coding: utf-8 -*-
import socket

files = ['file1', 'file2', 'file3']
IP = '127.0.0.1'
PORT = 2222

server = socket.socket()
server.connect((IP, PORT))

import time

for file in files:
    server.send(file.encode('utf-8'))
    time.sleep(2)
    with open(file, 'rb') as content:
        data = content.read()
    server.sendall(data)
    server.send(b'\x00')
    time.sleep(5)
server.close()