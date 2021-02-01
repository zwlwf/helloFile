#ifndef _COMMON_H
#define _COMMON_H
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

const char PUSHCHAR = '1';
const char PULLCHAR = '0';

int receiveInt(int sock) {
	int n;
	read(sock, (void*)&n, sizeof(int));
	return ntohl( n );
}

void* readBlock(int sock, int len) {
	void *buffer = malloc(len);
	void *p = buffer;
	while(len>0) {
		int rsize = read(sock, p, len);
		len-=rsize;
		p+=rsize;
	}
	return buffer;
}

int sendInt( int sock, int a) {
	int b = htonl(a);
	return send(sock, (void*)&b, sizeof(int),  MSG_DONTWAIT);
}

int sendBlock(int sock, void*block, int  len) {
	// some work may do for optimization
	return send(sock, block, len, 0);
}

typedef struct {
	int size;
	void* block;
} Package;

void sendPackage( int sock, Package p ) {
	sendInt(sock, p.size);
	if( p.size!=0 )
	{
		sendBlock(sock, p.block, p.size);
		free(p.block); 
	}
}

Package recvPackage( int sock) {
	Package ans;
	ans.size = receiveInt(sock);
	ans.block = readBlock(sock, ans.size);
	printf("received %d bytes data",ans.size);
	return ans;
}

#endif
