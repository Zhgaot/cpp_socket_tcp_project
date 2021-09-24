#include "basic_tcp.h"
#include "config.hpp"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>

using namespace std;

/* CONFIG */
const bool REPLY
  = Config::getInstance()->conf_msg["client"]["reply"].as<bool>();
const unsigned short DEFAULT_PORT
  = Config::getInstance()->conf_msg["client"]["default_port"].as<unsigned short>();
const long CONNECT_TIMEOUT
  = Config::getInstance()->conf_msg["client"]["connect_timeout"].as<long>();
const long RECV_BUF_LEN
  = Config::getInstance()->conf_msg["client"]["recv_buf_len"].as<long>();
const long SEND_BUF_LEN
  = Config::getInstance()->conf_msg["client"]["send_buf_len"].as<long>();

/**
 * @brief 主线程入口函数
 * @param argc 传入参数的个数
 * @param argv argv[0]:程序运行的全路径名; argc[1]:传入的参数(主要用于传入端口号)
*/
int main(int argc, char *argv[]) {

  /* 解析传入的ip地址和port端口 */
  string ip;
  unsigned short port = DEFAULT_PORT; // 默认端口号为8080
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
  client.create_connection(ip.c_str(), port, CONNECT_TIMEOUT);

  if (REPLY) {
    string send_msg_input = "";
    cout << "Please enter the reply information: ";
    cin >> send_msg_input;
    char send_msg[SEND_BUF_LEN];
    strcpy(send_msg, send_msg_input.c_str());
    int send_size = strlen(send_msg);
    int send_len = client.send_msg(send_msg, send_size);
  } else {
    client.send_msg("I am client.", 13);
  }

  char recv_buf[RECV_BUF_LEN];
  client.receive_msg(recv_buf, sizeof(recv_buf));
  cout << "server: " << recv_buf << endl;
  return 0;
}