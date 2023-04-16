#!/usr/bin/env python3
import socket

Host = '192.168.122.14'     # board
#Host = '192.168.122.154'   # windows(Notebook 2)
#Host = '192.168.122.124'   # VMWare
#Host = '127.0.0.1'         # loopback
#Host = '192.168.122.107'   # windows(Notebook 1)
Port = 333

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

client_socket.connect((Host, Port))
print("wait...")

'''
while True:
    # data send
    data = input(">>")
    client_socket.sendall(data.encode())
    # data received
    res = client_socket.recv(1024).decode().replace("\x00", "")

    if res == 'bye':
        client_socket.close()
        break
    print('Received', repr(res))
'''

ok = client_socket.recv(1024).decode().replace("\x00", "")
print('Received', repr(ok))

'''
while True:
    # ok received
    ok = client_socket.recv(1024).decode().replace("\x00", "")
    if ok == 'ok':
        # data send
        data = input(">>")
        client_socket.sendall(data.encode())
        # data received
        res = client_socket.recv(1024).decode().replace("\x00", "")

        if res == 'bye':
            client_socket.close()
            break
        print('Received', repr(res))
'''


client_socket.close()