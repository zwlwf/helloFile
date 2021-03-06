#ifndef _SERVER_H
#define _SERVER_H
#include "common.h"

int createSock_srv(){ 
	int sock = socket( AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr) );
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_ANY );
	addr.sin_port = htons( port );
	//addr.sin_port = htons( readPort() );
	int err = bind(sock, (struct sockaddr*)&addr, sizeof(addr) );
	if(err<0) {
		printf("bind error \n");
		exit(-1);
	}
	err = listen(sock, 20); 
	if(err<0) {
		printf("listen error \n");
		exit(-1);
	}
	return sock;
}

int waitConnect( int sock ) {
	struct sockaddr_in client_addr;
	int client_addr_size;
	int client_sock = -1;
	while(1) {
		client_sock = accept(sock, (struct sockaddr*)&client_addr, &client_addr_size);
		if( client_sock >=0 ) break;
	}
	char buffer[1024];
	char buffer2[2024];
	inet_ntop(AF_INET, &client_addr.sin_addr, buffer, sizeof(buffer));
	sprintf(buffer2," visitor: %s",buffer);
	Log(buffer2);
	return client_sock;
}

char readHello( int client_sock ) {
	char s[100]={0};
	time_t pre = time(NULL);
	while(1) {
		int t = recv(client_sock, s, 5, MSG_DONTWAIT);
		if(t>0) 
			break;
		else {
			time_t now = time(NULL);
			if(now-pre>3) // wait at most 3 seconds
				break;
		}
	}
	if(strcmp(s, "@PUSH")==0) return PUSHCHAR;
	else if(strcmp(s,"@PULL")==0) return PULLCHAR;
	else return '2';
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
