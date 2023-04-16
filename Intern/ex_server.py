#!/usr/bin/env python3

import socket

Host = '192.168.122.107'
#Host = '127.0.0.1'
Port = 9999

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind((Host, Port))
print('wait...')
print("[*] Started listening on ", Host, ":", Port)
server_socket.listen()
client_socket, addr = server_socket.accept()
print('Connected by', addr)

# board
while True:
    data = client_socket.recv(1024)
    msg = data.decode().replace("\x00", "")
    print("received message :", msg)
    client_socket.sendall(data)
    if msg == 'bye':
        client_socket.close()
        break

server_socket.close()

'''
# computer
while True:
    data = client_socket.recv(1024)
    msg = data.decode().replace("\x00", "")
    print("received message :", msg)
    client_socket.sendall(data)
    
    if msg == 'bye':
        client_socket.close()
        break

server_socket.close()
'''
