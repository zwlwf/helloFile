#include "client.h"

int main(int argc, char** argv) {
	char flag = dealWithInput( argc, argv); //从输入判断是pull还是push
	int sock = createSock_client(); // 连接到服务器的sock, 需要读配置文件
	if ( flag == PUSHCHAR ) {
		sayHello(sock, PUSHCHAR); // 先发给招呼, 要push
		Package p = tar(argc-2, argv+2); // 将要发送的内容打包
		sendPackage(sock, p); // 发包
		printf("------- push over ------- \n");
	} else if (flag==PULLCHAR) {
		sayHello(sock, PULLCHAR); // 先发给招呼，要pull
		Package p = recvPackage(sock); // 收包
		untar(p); // 解压包，进一步处理
		//printf("------- pull over ------- \n");
	}
	close(sock);
}
