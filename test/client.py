#Python 3
#Usage: python3 client.py [pull/push data]
#coding: utf-8
from socket import *
import sys

#Server would be running on the same host as Client
serverName = 'aliyun'
serverPort = 6667
PUSHCHAR = '1'.encode()
PULLCHAR = '0'.encode()

clientSocket = socket(AF_INET, SOCK_STREAM)

def tar( int_array ) :
    # send an int array with size n, to server
    n = len(int_array)
    block = n.to_bytes(4, byteorder='big')
    for x in int_array:
        block += x.to_bytes(4, byteorder='big')
    return len(block).to_bytes(4, byteorder='big') + block

def untar( p ) :
    # p[0] is length of block, useless here
    # block = [n, arr]
    block = p[1]
    n = int.from_bytes( block[:4], 'big')
    ans = []
    for i in range(4,len(block),4):
        ans.append( int.from_bytes( block[i:i+4], 'big') )
    return ans

def recvPackage( sock ) :
    size = int.from_bytes(sock.recv(4), 'big')
    block = sock.recv(size)
    return size, block

clientSocket.connect( (serverName, serverPort) )

if sys.argv[1] == 'pull':
    clientSocket.send(PULLCHAR)
    pkg = recvPackage(clientSocket)
    print( untar(pkg) )
elif sys.argv[1] == 'push': 
    clientSocket.send(PUSHCHAR)
    clientSocket.send(tar(list(range(10))))
    print( 'sending data done' )

# Close the socket
clientSocket.close()
