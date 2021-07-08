#include "basic_tcp.h"
#include "echo_service.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <thread>
using namespace std;

int main(int argc, char *argv[])
{
	/**
	 * @brief 主函数
	 * @param argc 传入参数的个数
	 * @param argv 第0个参数为“程序运行的全路径名”，从第1个参数开始为传入的参数(主要用于传入端口号)
	*/

	unsigned short port = 8080; // 默认端口号为8080
	if (argc > 1)
	{
		port = atoi(argv[1]); // atoi()函数将数字格式的字符串转换为整数类型，需要引用头文件<stdlib.h>
	}

	BasicTcp server(port);
	server.create_socket();
	server.name_socket();
	server.server_listen(10);

	/* 循环接受连接(支持多个客户端同时连接到服务端)，并获取客户端的信息 */
	while (true)
	{
		BasicTcp client = server.accept_connection();
		/* 创建一个线程专门用于客户端与服务端之间收发数据 */
		TcpRecvSendThread *cur_rs_thread = new TcpRecvSendThread(client); // 只有当前线程是new出来的，才能在类中使用delete this;释放空间
		bool reply = false;
		while (true)
		{
			cout << "Please select whether you need to manually reply to the information(y/n):";
			string reply_choose;
			cin >> reply_choose;
			if (reply_choose == "y")
			{
				reply = false;
				break;
			}
			else if (reply_choose == "n")
			{
				reply = true;
				break;
			}
			else
			{
				cout << "Please enter (y/n)!" << endl;
				continue;
			}
		}
		thread cur_thread(&TcpRecvSendThread::recv_send, std::ref(cur_rs_thread), reply);
		cur_thread.detach();
	}
	server.close_socket();

	return 0;
}
