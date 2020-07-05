# -*- coding: utf-8 -*-

import socket

conn = socket.socket()
IP = '127.0.0.1'
PORT = 2222
conn.bind((IP, PORT))
conn.listen(1)

client, addr = conn.accept()

for i in range(3):
    file = client.recv(256).decode('utf-8')
    with open(file, '+ab') as file:
        while True:
            data = client.recv(4096)
            if b'\x00' in data:
                file.write(data[:-1])
                break
            elif not data:
                break
            file.write(data)

client.close()
