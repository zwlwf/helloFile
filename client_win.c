#include <WinSock2.h>
#include <windows.h>
#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "6666"
typedef unsigned int uint32_t;

const int MSG_DONTWAIT = 0;
typedef struct _fdata {
	uint32_t nameSize;
	uint32_t dataSize;
} fdata;

static int receiveInt(int client_sock) {
	int n;
	recv(client_sock, (void*)&n, sizeof(int),0);
	return ntohl( n );
}

static void* readBlock(int client_sock, int len) {
        void *buffer = malloc(len);
        void *p = buffer;
        while(len>0) {
                int rsize = recv(client_sock, p, len, 0);
                len-=rsize;
                p+=rsize;
        }
        return buffer;
}

void pull(int sock) {
	// 想拉东西前，跟服务器打个招呼
	send(sock, "0", 1, MSG_DONTWAIT);

    int bufferSize = receiveInt(sock);
	void* block = readBlock(sock, bufferSize);
	uint32_t fnum;
	uint32_t pos = 0;
	memcpy((void *)&fnum, block, sizeof(uint32_t));
	fnum = ntohl(fnum);
	pos+=sizeof(uint32_t);
	printf(" files number = %u \n", fnum);
	fdata fd;
	char fname[200];
	for(int i=0; i<fnum; i++) {
		memcpy((void*)&fd, block+pos, sizeof(fdata));
		pos+=sizeof(fdata);
		fd.nameSize = ntohl(fd.nameSize);
		fd.dataSize = ntohl(fd.dataSize);
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
		pos+=fd.dataSize;
	}
	free(block);
	printf("------- pull over ------- \n");
}

int push(int sock, int n, char** fnames) {
	unsigned char *buffer;
	size_t bufferSize = sizeof(uint32_t); // the first data is the number fof files
	uint32_t fnum = n;
	uint32_t* fSize = malloc(sizeof(uint32_t)*fnum);
	for(int i=0; i<n; i++) {
		FILE* fp = fopen(fnames[i],"rb");
		if(!fp) {
			printf("file %s not exist\n", fnames[i]);
			return -1;
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

	uint32_t tn = htonl( bufferSize);
	send(sock, "1", 1, MSG_DONTWAIT);
	send(sock, (void*)&tn , sizeof(int), MSG_DONTWAIT);
	int nWrite = send(sock, buffer, bufferSize, 0 );
	free(buffer);
	free(fSize);

}

void helper() {
	printf("Useage : pushFile file1 [file2, [file3..]] \n");
}

void work(void* block) {
	uint32_t fnum;
	uint32_t pos = 0;
	memcpy((void *)&fnum, block, sizeof(uint32_t));
	fnum = ntohl(fnum);
	printf("file numbers = %u\n", fnum);
	if(fnum>10) return;
	pos+=sizeof(uint32_t);
	fdata fd;
	char fname[200];
	for(int i=0; i<fnum; i++) {
		memcpy((void*)&fd, block+pos, sizeof(fdata));
		pos+=sizeof(fdata);
		fd.nameSize = ntohl(fd.nameSize);
		fd.dataSize = ntohl(fd.dataSize);
		memcpy(fname, block+pos, fd.nameSize);
		fname[fd.nameSize] = '\0';
		printf("reading file: %s\n", fname);
		pos+=fd.nameSize;
		// write the data of bloc to file?
		FILE *fp = fopen(fname, "wb");
		if(fp) {
			fwrite(block+pos, fd.dataSize, sizeof(void), fp);
		}
		else {
			printf("can not open file for writing\n");
		}
		fclose(fp);
		pos+=fd.dataSize;
	}
}

int main(int argc, char** argv)
{
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		   *ptr = NULL,
		   hints;
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_INET;  //可以是IPv4或IPv6地址
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	iResult = getaddrinfo("47.104.98.157", DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	
	ptr = result;
	// create socket
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
		ptr->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
		printf("connection failed \n");
		return 1;
	}

	if ( strcmp(argv[1], "push") == 0) 
		push(ConnectSocket, argc-2, argv+2);
	else if ( strcmp( argv[1], "pull") == 0) 
		pull(ConnectSocket);
	else 
		helper();

	return 0;
}

