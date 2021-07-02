#ifdef WIN32
#include <windows.h>  // 在Windows下需导入但在Linux下无需导入的头文件
#else
#include <sys/types.h>  // Linux下编译需要导入的头文件
#include <sys/socket.h>
#include <unistd.h>  // close函数需要导入的头文件
#define closesocket close  // Linux下关闭socket需要使用close，因此在Linux下将closesocket替换为close
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#endif
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <errno.h>
#define INET_ADDRSTRLEN 16
using namespace std;

void byteorder() {
	union {
		short value;
		char union_bytes[sizeof(short)];
	} test;
	test.value = 0x0102;
	if ((test.union_bytes[0] == 1) && (test.union_bytes[1] == 2)) {
		cout << "big endian" << endl;
	}
	else if ((test.union_bytes[0] == 2) && (test.union_bytes[1] == 1)) {
		cout << "little endian" << endl;
	}
	else {
		cout << "unknow..." << endl;
	}
}

class TcpRSThread {
public:
	TcpRSThread(int client_sock) : _client_sock(client_sock) {}
	void Main() {
		/* 循环读取客户端数据，并给予回复 */
		char buf[1024] = {0};
		while (true) {
			int recvlen = recv(_client_sock, buf, sizeof(buf)-1, 0);  // recvlen表示实际返回值
			if (recvlen <= 0)
				break;
			else
				buf[recvlen] = '\0';  // 在字符串结尾
			if (strstr(buf, "quit") != nullptr) {
				char quitRes[] = "quit success!\n";
				send(_client_sock, quitRes, sizeof(quitRes)+1, 0);
				break;
			}
			cout << "读取客户端传入的消息并打印输出: " << buf << endl;
			int sendlen = send(_client_sock, "OK\n", 4, 0);
		}
		closesocket(_client_sock);
		delete this;
	}
private:
	int _client_sock = 0;
};

int main(int argc, char* argv[]) {  // argv用于传入端口号

	/* 初始化动态链接库 */
#ifdef WIN32  // 在Linux中无需初始化动态链接库
    static bool first = true;  // 首次初始化为true，也就是需要进行初始化
    if (first == true) {
        WSADATA ws;
	    WSAStartup(MAKEWORD(2, 2), &ws);  // 2,2表示版本号
        first = false;
    }
#endif
	
	/* 创建socket描述符(这个socket描述符是服务端用来接收连接的)，并作失败判断 */
	//int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // 使用IPPROTO_TCP在Linux中编译不通过
	int sock = socket(PF_INET, SOCK_STREAM, 0);  // PF_INET(IPv4网络协议中采用的协议族)，SOCK_STREAM(TCP/IP协议)
	if (sock == -1) {
		cout << "创建socket失败！" << endl;
		return -1;
	}
	else
		cout << "socket描述符为：" << sock << endl;

	/* 使用TCP/IP协议的<专用socket地址>来设置地址族、端口号、IP地址 */
	unsigned short port = 8080;
	if (argc > 1) {
		port = atoi(argv[1]);  // atoi()函数将数字格式的字符串转换为整数类型
	}
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;  // AF_INET是地址族，对应于IPv4网络协议中采用的协议族PF_INET
	saddr.sin_port = htons(port);  // htons()函数将短整型的主机字节序数据转化为网络字节序数据，通常用于转换端口号
	saddr.sin_addr.s_addr = htons(0);  // 设置IPv4地址(要用网络字节序表示)，设置为0表示任意ip地址发过来的数据均可接收

	/* 命名socket：即将一个socket与socket地址绑定 */
	if (bind(sock, (sockaddr*)&saddr, sizeof(saddr)) != 0) {
#ifdef WIN32
		cout << "命名(绑定)socket失败！绑定的端口号为：" << port << "；"
			<< "失败原因为：" << GetLastError() << endl;  // Windows下使用GetLastError()获得错误码
#else
		cout << "命名(绑定)socket失败！绑定的端口号为：" << port << "；" 
			<< "失败原因为：" << strerror(errno) << endl;  // Linux下使用strerror解读errno来得知为何绑定失败
#endif
		return -2;
	}
	else {
		cout << "命名(绑定)socket成功！绑定的端口号为：" << port << endl;
	}

	/* 监听socket */
	int listen_res = listen(sock, 10);  // 第二个参数backlog表示内核监听队列的最大长度，若监听队列长度超过此值，服务器将不再受理新的客户连接
	if (listen_res == 0) {  // listen成功则返回0
		cout << "服务端已开始监听......" << endl;
	}
	else {
		cout << "服务端监听失败！" << endl;
	}

	/* 循环接受连接(支持多个客户端同时连接到服务端)，并获取客户端的信息 */
	while (true) {
		sockaddr_in caddr;  // 该socket地址用于存储客户端的信息
		socklen_t len = sizeof(caddr);
		int client_sock = accept(sock, (sockaddr*)&caddr, &len);
		// int client_sock = accept(sock, (sockaddr*)&caddr, sizeof(caddr));
		if (client_sock < 0) {  // 接受客户端连接失败
			cout << "接受客户端连接失败，error is：" << strerror(errno) << endl;
			continue;
		}
		else {  // 接受客户端连接成功
			char remote[INET_ADDRSTRLEN];
			const char* c_ip = inet_ntop(AF_INET, &caddr.sin_addr, remote, INET_ADDRSTRLEN);  //《Linux高性能服务器编程》p74&79
			unsigned short c_port = ntohs(caddr.sin_port);  // 获取端口号(unsigned short表明了端口号最多为65535)
			cout << "连接成功！\n"
			<< "客户端的ip地址为：" << c_ip
			<< "客户端的端口号为：" << c_port << endl;
		}

		/* 创建一个线程专门用于客户端与服务端之间收发数据 */
		TcpRSThread* cur_rs_thread = new TcpRSThread(client_sock);  // 只有当前线程是new出来的，才能在类中使用delete this;释放空间
		thread cur_thread(&TcpRSThread::Main, cur_rs_thread);
		cur_thread.detach();
	}

	/* 在所有操作后再关闭socket */
	closesocket(sock);  // closesocket是Windows下的语句，顶部的define宏定义了linux下关闭socket的语句，即：close

	return 0;
}
