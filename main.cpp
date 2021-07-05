#include "XTcp.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <thread>
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
	TcpRSThread(XTcp client) : _server_client(client) {}
	void Main() {
		/* 循环读取客户端数据，并给予回复 */
		char recv_buf[1024] = {0};
		while (true) {
			int recvlen = _server_client.receive_msg(recv_buf, sizeof(recv_buf) - 1);  // recvlen表示实际返回值
			if (recvlen <= 0)
				break;
			else
				recv_buf[recvlen] = '\0';  // 在字符串结尾
			if (strstr(recv_buf, "quit") != nullptr) {  // 当客户端需要退出时
				char send_msg[] = "quit success!\n";
				_server_client.send_msg(send_msg, sizeof(send_msg) + 1);
				break;
			}
			cout << "The information sent by the client is: " << recv_buf << endl;
			char send_msg[] = "OK\n";
			int sendlen = _server_client.send_msg(send_msg, sizeof(send_msg));
		}
		_server_client.close_socket();
		delete this;
	}
private:
	XTcp _server_client;
};

int main(int argc, char* argv[]) {
	/**
	 * @brief 主函数
	 * @param argc 传入参数的个数
	 * @param argv 第0个参数为“程序运行的全路径名”，从第1个参数开始为传入的参数(主要用于传入端口号)
	*/

	unsigned short port = 8088;  // 默认端口号为8080
	if (argc > 1) {
		port = atoi(argv[1]);  // atoi()函数将数字格式的字符串转换为整数类型，需要引用头文件<stdlib.h>
	}

	XTcp server;
	server.create_socket();
	server.name_socket(port);
	server.server_listen(10);

// 	/* 初始化动态链接库 */
// #ifdef WIN32  // 在Linux中无需初始化动态链接库
//     static bool first = true;  // 首次初始化为true，也就是需要进行初始化
//     if (first == true) {
//         WSADATA ws;
// 	    WSAStartup(MAKEWORD(2, 2), &ws);  // 2,2表示版本号
//         first = false;
//     }
// #endif
	
	// /* 创建socket描述符(这个socket描述符是服务端用来接收连接的)，并作失败判断 */
	// //int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // 使用IPPROTO_TCP在Linux中编译不通过
	// int sock = socket(PF_INET, SOCK_STREAM, 0);  // PF_INET(IPv4网络协议中采用的协议族)，SOCK_STREAM(TCP/IP协议)
	// if (sock == -1) {
	// 	cout << "创建socket失败！" << endl;
	// 	return -1;
	// }
	// else
	// 	cout << "socket描述符为：" << sock << endl;

// 	/* 使用TCP/IP协议的<专用socket地址>来设置地址族、端口号、IP地址 */
// 	unsigned short port = 8080;
// 	if (argc > 1) {
// 		port = atoi(argv[1]);  // atoi()函数将数字格式的字符串转换为整数类型
// 	}
// 	sockaddr_in saddr;
// 	saddr.sin_family = AF_INET;  // AF_INET是地址族，对应于IPv4网络协议中采用的协议族PF_INET
// 	saddr.sin_port = htons(port);  // htons()函数将短整型的主机字节序数据转化为网络字节序数据，通常用于转换端口号
// 	saddr.sin_addr.s_addr = htons(0);  // 设置IPv4地址(要用网络字节序表示)，设置为0表示任意ip地址发过来的数据均可接收

// 	/* 命名socket：即将一个socket与socket地址绑定 */
// 	if (bind(sock, (sockaddr*)&saddr, sizeof(saddr)) != 0) {
// #ifdef WIN32
// 		cout << "命名(绑定)socket失败！绑定的端口号为：" << port << "；"
// 			<< "失败原因为：" << GetLastError() << endl;  // Windows下使用GetLastError()获得错误码
// #else
// 		cout << "命名(绑定)socket失败！绑定的端口号为：" << port << "；" 
// 			<< "失败原因为：" << strerror(errno) << endl;  // Linux下使用strerror解读errno来得知为何绑定失败
// #endif
// 		return -2;
// 	}
// 	else {
// 		cout << "命名(绑定)socket成功！绑定的端口号为：" << port << endl;
// 	}

	// /* 监听socket */
	// int listen_res = listen(sock, 10);  // 第二个参数backlog表示内核监听队列的最大长度，若监听队列长度超过此值，服务器将不再受理新的客户连接
	// if (listen_res == 0) {  // listen成功则返回0
	// 	cout << "服务端已开始监听......" << endl;
	// }
	// else {
	// 	cout << "服务端监听失败！" << endl;
	// }

	/* 循环接受连接(支持多个客户端同时连接到服务端)，并获取客户端的信息 */
	while (true) {
		XTcp client = server.accept_connection();
		/* 创建一个线程专门用于客户端与服务端之间收发数据 */
		TcpRSThread* cur_rs_thread = new TcpRSThread(client);  // 只有当前线程是new出来的，才能在类中使用delete this;释放空间
		thread cur_thread(&TcpRSThread::Main, std::ref(cur_rs_thread));
		cur_thread.detach();
	}
	server.close_socket();

	return 0;
}
