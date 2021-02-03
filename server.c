#include "server.h"

int main() {
	int sock = createSock_srv(); // 设置sock，默认bind INADDR_ANY
	queue *Q = (queue*) malloc(sizeof(queue)); // 申请一个队列保存收集的package
	queue_init(Q, 20);
	while(1) {
		int client_sock = waitConnect(sock); // 等待client连接
		char flag = readHello(client_sock);  // 判断client发送的头
		printf("Read request : ->%s <--\n", flag==PUSHCHAR? "push" : "pull");
		if (flag==PULLCHAR) {
			sendPackage( client_sock, queue_pop(Q) ); // 发包
		} else if (flag==PUSHCHAR) {
			queue_push( Q, recvPackage(client_sock) ); // 收包
		} else {
			printf("Unknown request\n");
		}
		close(client_sock);
	}
	close(sock);
}
