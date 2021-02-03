#ifndef _SERVER_H
#define _SERVER_H
#include "common.h"

int createSock_srv(){ 
	int sock = socket( AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_ANY );
	addr.sin_port = htons( 6667 );
	bind(sock, (struct sockaddr*)&addr, sizeof(addr) );
	listen(sock, 20); 
	return sock;
}

int waitConnect( int sock ) {
	struct sockaddr_in client_addr;
	int client_addr_size;
	int client_sock = accept(sock, (struct sockaddr*)&client_addr, &client_addr_size);
	return client_sock;
}

char readHello( int client_sock ) {
	char c;
	int t = recv(client_sock, (void*)&c, 1, 0);
	return c;
}

typedef struct _queue {
	int l;
	int r;
	int size;
	int capacity;
	Package* data;
} queue;

void queue_init(queue*q, int n) {
	q->data = malloc(n*sizeof(Package));
	q->l = 0;
	q->r = -1;
	q->size = 0;
	q->capacity = n;
}

int queue_push(queue* q, Package p) {
	if(q->size >= q->capacity) return -1; // full
	++q->r;
	if(q->r >= q->capacity) q->r%=q->capacity;
	q->data[q->r] = p;
	q->size++;
	return 0;
}

Package queue_pop(queue* q) {
	if(q->size==0) {
		Package a;
		a.size = 0;
		a.block = NULL;
		return a;
	}
	Package ans = q->data[q->l];
	q->l++;
	if(q->l >= q->capacity) q->l%=q->capacity;
	q->size--;
	return ans;
}

void queue_release(queue* q) {
	free(q->data);
}

#endif
