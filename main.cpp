#include "XTcp.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <thread>
using namespace std;

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

	unsigned short port = 8080;  // 默认端口号为8080
	if (argc > 1) {
		port = atoi(argv[1]);  // atoi()函数将数字格式的字符串转换为整数类型，需要引用头文件<stdlib.h>
	}

	XTcp server(port);
	server.create_socket();
	server.name_socket();
	server.server_listen(10);

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
