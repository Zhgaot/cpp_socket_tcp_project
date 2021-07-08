#include "basic_tcp.h"
#include <iostream>
#include <stdlib.h>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
  /* 解析传入的ip地址和port端口 */
  string ip;
  unsigned short port = 8080; // 默认端口号为8080
  if (argc > 2) {
    ip = argv[1];
    port = atoi(argv[2]);
  } else {
    cout << "Please enter the ip address and port in the terminal!" << endl;
    return -1;
  }

  /* 创建客户端，并建立连接 */
  BasicTcp client;
  client.create_socket();
  client.set_block(false);
  client.create_connection(ip.c_str(), port);
  client.send_msg("I am client.", 13);
  char recv_buf[2014];
  client.receive_msg(recv_buf, sizeof(recv_buf));
  cout << recv_buf << endl;
  return 0;
}