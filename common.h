#ifndef _COMMON_H
#define _COMMON_H
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#include "config.h"
const char PUSHCHAR = '1';
const char PULLCHAR = '0';
const int MAXSIZE = 300<<20; // Set max size of file to 300M

void Log(const char* message) {
	FILE *fp = fopen("helloFile.log","a+");
	char ts[100];
	time_t now = time(NULL);
	strftime(ts, 100, "%D %H:%M:%S", localtime(&now));
	fprintf(fp, "[%s ]%s\n",ts, message);
	fclose(fp);
}

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
		printf("rsize = %d\n",rsize);
		len-=rsize;
		p+=rsize;
	}
	return buffer;
}

int sendInt( int sock, int a) {
	int b = htonl(a);
	return send(sock, (void*)&b, sizeof(int),  0);
}

int sendBlock(int sock, void*block, int  len) {
	// some work may do for optimization
	while(len>0) {
		int wsize = send(sock, block, len, 0);
		len -= wsize;
		block += wsize;
	}
	return 0;
}

typedef struct {
	int size;
	void* block;
} Package;

void sendPackage( int sock, Package p ) {
	printf("sending %d bytes\n", p.size);
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
	if(ans.size <= 0) {
		Log("Error : package with negetive size ignored!\n");
		ans.size = 0;
		ans.block = NULL;
		return ans;
	} else if( ans.size> MAXSIZE) {
		Log("Error : package with too large size ignored!\n");
		ans.size = 0;
		ans.block = NULL;
		return ans;
	}
	printf("receiving %d bytes data\n",ans.size);
	ans.block = readBlock(sock, ans.size);
	printf("received %d bytes data\n",ans.size);
	return ans;
}

#define configFile "config"

void readIP(char* s) {
	FILE *fp = fopen(configFile,"r");
	if(!fp) {
		printf("Failed to read config\n");
		exit(-1);
	}
	char line[512]={0};
	char pattern[100] = "IP";
	while( fgets(line, 512, fp) != NULL ) {
		int i=0;
		int j=0;
		while(line[i]==' ') i++;
		while( pattern[j] && line[i] && pattern[j]== toupper(line[i]) ) {
			i++;
			j++;
		}
		if(!pattern[j]) {
			//while( !(isdigit(line[i]) || line[i]=='.') ) i++;
			//while( line[i] && (isdigit(line[i]) || line[i]=='.')) *s++ = line[i++];
			while( line[i]==' ' || line[i]=='=' ) i++;
			while( line[i] && !isspace(line[i]) ) *s++ = line[i++];
			break;
		}
	}
	*s = 0;
	fclose(fp);
}

short readPort() {
	FILE *fp = fopen(configFile,"r");
	if(!fp) {
		printf("Failed to read config\n");
		exit(-1);
	}
	char line[512]={0};
	char pattern[20] = "PORT";
	int ans = 0;
	while( fgets(line, 512, fp) != NULL ) {
		int i=0;
		int j=0;
		while(line[i]==' ') i++;
		while( pattern[j] && line[i] && pattern[j]== toupper(line[i]) ) {
			i++;
			j++;
		}
		if(!pattern[j]) {
			while( !isdigit(line[i]) ) i++;
			while( line[i] && isdigit(line[i])) ans = ans*10 + (line[i++]-'0');
			break;
		}
	}
	fclose(fp);
	return ans;
}
#endif
