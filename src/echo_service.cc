#include "echo_service.h"
#include <iostream>
#include <string.h>
#include <string>
using namespace std;

TcpRecvSendThread::TcpRecvSendThread(BasicTcp client)
    : server_client_(client) {}

void TcpRecvSendThread::recv_send(bool reply) {
  /* 循环读取客户端数据，并给予回复 */
  char recv_buf[1024] = {0};
  while (true) {
    // recvlen表示实际返回值
    int recvlen = server_client_.receive_msg(recv_buf, sizeof(recv_buf) - 1);
    // 若recv函数出错则直接跳出循环，关闭此链接
    if (recvlen <= 0) {
      cout << "An error occurred in the recv function!" << endl;
      break;
    } else
      recv_buf[recvlen] = '\0';
    if (strstr(recv_buf, "quit") != nullptr ||
        strstr(recv_buf, "exit") != nullptr) // 当客户端需要退出时
    {
      char send_msg[] = "quit success!\n";
      server_client_.send_msg(send_msg, sizeof(send_msg) + 1);
      cout << "A client has exited~~~" << endl;
      break;
    }
    cout << "The information sent by the client is: " << recv_buf << endl;
    if (reply) {
      string send_msg_input = "";
      cout << "Please enter the reply information: ";
      cin >> send_msg_input;
      char send_msg[1024];
      strcpy(send_msg, send_msg_input.c_str());
      int sendlen = server_client_.send_msg(send_msg, sizeof(send_msg));
    } else {
      char send_msg[] = "OK\n";
      int sendlen = server_client_.send_msg(send_msg, sizeof(send_msg));
    }
  }
  server_client_.close_socket();
  delete this;
}
