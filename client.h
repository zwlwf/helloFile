#ifndef _CLIENT_H
#define _CLIENT_H
#include "common.h"

typedef struct _fdata {
	uint32_t nameSize;
	uint32_t dataSize;
} fdata;

static void helper() {
	printf("Useage : helloFile push file1 [file2, [file3..]] \n" 
	       "      or helloFile pull\n");
}

int createSock_client() {
	int sock = socket( AF_INET, SOCK_STREAM, 0);
	struct addrinfo *result = NULL,hints;
	
	// // read ip and port from config
	//short port=6667;
	//char ipstr[20];
	//readIP(ipstr);
	//port = readPort();
	//printf("Ip = %s , port = %d", ipstr, port);
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int iresult = getaddrinfo(ipstr, portstr, &hints, &result);
	
	if(iresult!=0 ){
		printf("getaddrinfo fault\n");
		return -1;
	}
	int flag = connect(sock, (struct sockaddr*)(result->ai_addr), sizeof(struct sockaddr) );
	if(flag ){
		printf("connection failed \n");
		return -1;
	}
	return sock;
}

char dealWithInput(int argc, char** argv) {
	if(argc<2) {
		helper();
		exit(-1);
	}
	if( strcmp(argv[1], "pull") == 0) return PULLCHAR;
	else  return PUSHCHAR;
}

int sayHello( int sock, char c ) {
	char s[100];
	if(c==PUSHCHAR) 
		strcpy(s,"@PUSH");
	else
		strcpy(s,"@PULL");
	return send(sock, s, strlen(s), 0);
}

unsigned char* readUntilEOF() {
    int data_length=0;
    int alloc_length = 1024;
    char *data = (char*) malloc( alloc_length );
    char c;
    while( ( c = fgetc(stdin)) != EOF ) {
        if(data_length + 1 > alloc_length) {
            alloc_length <<=1;
            data = realloc(data, alloc_length);
        }
        data[data_length++] = c;
    }
    data[data_length] = '\0';
    return data;
}

Package tar_stdin() {
	unsigned char* buffer;
	uint32_t fnum = htonl(1);
	uint32_t fname_length = htonl(0);
	unsigned char* data = readUntilEOF();
	size_t data_length = strlen(data)+1;
	uint32_t data_length_send = htonl(data_length);
	size_t bufferSize = 3*sizeof(uint32_t) + data_length;
	buffer = (unsigned char*) malloc(bufferSize);
	memcpy(buffer, (const void*)&fnum, sizeof(uint32_t));
	memcpy(buffer+sizeof(uint32_t), (const void*)&fname_length, sizeof(uint32_t));
	memcpy(buffer+2*sizeof(uint32_t), (const void*)&data_length_send, sizeof(uint32_t));
	memcpy(buffer+3*sizeof(uint32_t), (const void*)data, data_length);
	free(data);
	Package ans;
	ans.size = bufferSize;
	ans.block = buffer;
	return ans;
}
	
Package tar(int n, char** fnames) {
	if(n==0) return tar_stdin();
	unsigned char *buffer;
	size_t bufferSize = sizeof(uint32_t); // the first data is the number fof files
	uint32_t fnum = n;
	uint32_t* fSize = malloc(sizeof(uint32_t)*fnum);
	for(int i=0; i<n; i++) {
		FILE* fp = fopen(fnames[i],"rb");
		if(!fp) {
			printf("file %s not exist\n", fnames[i]);
			exit(-1);
		}
		fseek(fp,0,SEEK_END);
		fSize[i] = ftell(fp);
		bufferSize+=sizeof(fdata) + strlen(fnames[i]) + fSize[i];
		fclose(fp);
	}
	// prepare buffer
	buffer = malloc(bufferSize);
	size_t pos = 0;
	uint32_t fnumnx = htonl(fnum);
	memcpy(buffer, (const void*)&fnumnx, sizeof(uint32_t));
	pos += sizeof(uint32_t);
	fdata fd;
	for(int i=0; i<n; i++) {
		FILE *fp = fopen(fnames[i],"rb");
		fd.nameSize = htonl(strlen(fnames[i]));
		fd.dataSize = htonl(fSize[i]);
		memcpy(buffer+pos, (const void*) &fd, sizeof(fdata));
		pos+=sizeof(fdata);
		memcpy(buffer+pos, fnames[i], strlen(fnames[i]));
		pos+=strlen(fnames[i]);
		fread( buffer+pos, fSize[i], sizeof(void), fp);
		pos+=fSize[i];
		fclose(fp);
	}
	Package ans;
	ans.size = bufferSize;
	ans.block = buffer;
	return ans;
}

void untar(Package p) {
	void *block = p.block;
	if(!block) {
		printf("no files in queue on server\n");
		return;
	}
	uint32_t fnum;
	uint32_t pos = 0;
	memcpy((void *)&fnum, block, sizeof(uint32_t));
	fnum = ntohl(fnum);
	pos+=sizeof(uint32_t);
	// printf(" files number = %u \n", fnum);
	fdata fd;
	char fname[200];
	for(int i=0; i<fnum; i++) {
		memcpy((void*)&fd, block+pos, sizeof(fdata));
		pos+=sizeof(fdata);
		fd.nameSize = ntohl(fd.nameSize);
		fd.dataSize = ntohl(fd.dataSize);
		if(fd.nameSize) {
			memcpy(fname, block+pos, fd.nameSize);
			fname[fd.nameSize] = '\0';
			printf("reading file: %s\n", fname);
			pos+=fd.nameSize;
			// write the data of bloc to file?
			FILE *fp = fopen(fname, "wb");
			if(fp) {
				fwrite(block+pos, fd.dataSize, sizeof(void), fp);
			}
			fclose(fp);
		} else { // untar the data from stdin
			printf("%s",(char*)block+pos);
			pos+=fd.dataSize;
		}
		pos+=fd.dataSize;
	}
	free(block);
}
#endif
