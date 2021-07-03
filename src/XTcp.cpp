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
#include "XTcp.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#include <errno.h>
#define INET_ADDRSTRLEN 16
using namespace std;

XTcp::XTcp() {
    /* 为了兼容windows下的使用，在XTcp类的构造函数内初始化动态链接库 */
#ifdef WIN32  // 在Linux中无需初始化动态链接库
    static bool first = true;  // 首次初始化为true，也就是需要进行初始化
    if (first == true) {
        WSADATA ws;
	    WSAStartup(MAKEWORD(2, 2), &ws);  // 2,2表示版本号
        first = false;
    }
#endif

    /* 初始化成员属性 */
    this->sock = -1;
    this->port = 0;
}

int XTcp::create_socket() {
    /* 创建socket描述符(这个socket描述符是服务端用来接收连接的)，并作失败判断 */
	//int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  // 使用IPPROTO_TCP在Linux中编译不通过
	this->sock = socket(PF_INET, SOCK_STREAM, 0);  // PF_INET(IPv4网络协议中采用的协议族)，SOCK_STREAM(TCP/IP协议)
	if (this->sock == -1) {
		cout << "Create socket failed!" << endl;
		return -1;
	}
	else
		cout << "The file descriptor of socket is: " << this->sock << endl;  // 输出socket描述符
}

bool XTcp::name_socket(unsigned short port) {
	/* 万一外部没有创建socket，在这里也创建上 */
	this->create_socket();

    /* 使用TCP/IP协议的<专用socket地址>来设置地址族、端口号、IP地址 */
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;  // AF_INET是地址族，对应于IPv4网络协议中采用的协议族PF_INET
	saddr.sin_port = htons(this->port);  // htons()函数将短整型的主机字节序数据转化为网络字节序数据，通常用于转换端口号
	saddr.sin_addr.s_addr = htons(0);  // 设置IPv4地址(要用网络字节序表示)，设置为0表示任意ip地址发过来的数据均可接收

	/* 命名socket：即将一个socket与socket地址绑定 */
	if (bind(this->sock, (sockaddr*)&saddr, sizeof(saddr)) != 0) {
#ifdef WIN32
		cout << "name(bind) socket failed！The port that you want to bind is: " << port << "；"
			<< "The reason for the failure is: " << GetLastError() << endl;  // Windows下使用GetLastError()获得错误码
#else
		cout << "name(bind) socket failed！The port that you want to bind is: " << port << "；" 
			<< "The reason for the failure is: " << strerror(errno) << endl;  // Linux下使用strerror解读errno来得知为何绑定失败
#endif
		return false;
	}
	else {
		cout << "name(bind) socket successfully! The port that you want to bind is: " << port << endl;
		return true;
	}
}

bool XTcp::server_listen(const int listen_num) {
	/* 监听socket */
	int listen_res = listen(this->sock, listen_num);  // 第二个参数backlog表示内核监听队列的最大长度，若监听队列长度超过此值，服务器将不再受理新的客户连接
	if (listen_res == 0) {  // listen成功则返回0
		cout << "The server has started listening......" << endl;
		return true;
	}
	else {
		cout << "Server listening failed!" << endl;
		return false;
	}
}

XTcp XTcp::accept_connection() {
	/**
	 * @return 返回的是一个从server角度看到的client，它包含有socket、ip、port
	*/
	XTcp temp_tcp;  // 创建一个XTcp对象，无论最终是否accept成功，都将其返回，外部可以通过XTcp对象内的sock进行判断是否accept成功
	sockaddr_in caddr;  // 该socket地址用于存储客户端的信息
	socklen_t len = sizeof(caddr);
	int client_sock = accept(sock, (sockaddr*)&caddr, &len);
	// int client_sock = accept(sock, (sockaddr*)&caddr, sizeof(caddr));
	if (client_sock < 0) {  // 接受客户端连接失败
		cout << "Failed to accept client connection，the reason for the failure is: " << strerror(errno) << endl;
	}
	else {  // 接受客户端连接成功
		char remote[INET_ADDRSTRLEN];
		temp_tcp.sock = client_sock;  // 将专门用于与客户端通信的socket描述符传入进去
		temp_tcp.ip = inet_ntop(AF_INET, &caddr.sin_addr, remote, INET_ADDRSTRLEN);  //《Linux高性能服务器编程》p74&79；另：const char* == std::string
		temp_tcp.port = ntohs(caddr.sin_port);  // 获取端口号(unsigned short表明了端口号最多为65535)
		cout << "connection succeeded！\n"
		<< "The ip of the client is" << temp_tcp.ip
		<< "The port of the client is" << temp_tcp.port << endl;
	}
	return temp_tcp;
}

void XTcp::close_socket() {
	/* 在所有操作后再关闭socket */
	if (this->sock == -1)
		return;
	closesocket(this->sock);  // closesocket是Windows下的语句，顶部的define宏定义了linux下关闭socket的语句，即：close
}

int XTcp::receive_msg(char* buf, int recv_size) {
	return recv(this->sock, buf, recv_size, 0);
}

int XTcp::send_msg(const char* buf, int send_size) {
	/**
	 * @brief 在send_msg中，外部指定了buf有多大(send_size)，就必须要保证发送多少的数据
	 * @param buf 外部传入的数据必须不可在函数内部修改，因此指定为const类型
	*/
	int sended_size = 0;  // 已发送的数据的大小
	while (sended_size != send_size) {
		int len = send(this->sock, buf + sended_size, send_size - sended_size, 0);  // 当前发送的起点：buf + sended_size；当前需要发送的大小send_size - sended_size
		if (len <= 0) {
			cout << "Failed to send data!" << endl;
			break;
		}
		sended_size += len;
	}
	return sended_size;
}
