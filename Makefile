all : client server

client : client.c client.h common.h
	gcc client.c -o helloFile -std=c99
server : server.c server.h common.h
	gcc server.c -o helloFile_server -std=c99
