all : client server

client : client.c client.h common.h
	gcc client.c -o ~/lab/helloFile -std=c99
server : server.c server.h common.h
	gcc server.c -o server -std=c99
