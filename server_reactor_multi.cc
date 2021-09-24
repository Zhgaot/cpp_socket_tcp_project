#include "basic_tcp.h"
#include "handler.h"
#include "thread_pool.hpp"
#include "reactor_mulit.hpp"
#include "config.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/epoll.h> // only Linux
#include <thread>
#include <vector>
#include <array>
#include <utility>

#define ARRAY 0

using namespace std;

/* CONFIG */
const int THREAD_NUM 
  = Config::getInstance()->conf_msg["reactor_multi"]["thread_quantity"].as<int>();
const int LISTEN_NUM
  = Config::getInstance()->conf_msg["reactor_multi"]["listen"].as<int>();
const bool REPLY
  = Config::getInstance()->conf_msg["reactor_multi"]["reply"].as<bool>();
const unsigned short DEFAULT_PORT
  = Config::getInstance()->conf_msg["reactor_multi"]["default_port"].as<unsigned short>();
const int EPOLL_CREATE_SIZE
  = Config::getInstance()->conf_msg["reactor_multi"]["epoll_create_size"].as<int>();
const int EPOLL_LISTEN_SIZE
  = Config::getInstance()->conf_msg["reactor_multi"]["epoll_listen_size"].as<int>();
const int EPOLL_TIMEOUT
  = Config::getInstance()->conf_msg["reactor_multi"]["epoll_timeout"].as<int>();
const string EPOLL_MODE
  = Config::getInstance()->conf_msg["reactor_multi"]["epoll_mode"].as<string>();

vector<ReactorMulitSub*> reactor_sub_list;

/* 源地址hash进行负载均衡，将不同Client分配到不同线程中 */
void call_back_main(const int client_fd, const std::string Client_ip,
                    const std::string event_mode) {
  reactor_sub_list[hash<string>()(Client_ip) % THREAD_NUM]->add_event(client_fd, event_mode);
}

void call_back_handler(const int fd) {
  BasicTcp client;
  client.sock = fd;
  RecvSendThread* handler = new RecvSendThread(client);
  handler->recv_send(false);
}

/**
 * @brief 主线程入口函数
 * @param argc 传入参数的个数
 * @param argv argv[0]:程序运行的全路径名; argc[1]:传入的参数(主要用于传入端口号)
*/
int main(int argc, char *argv[]) {

  /* 确定server端口号 */
  unsigned short port = DEFAULT_PORT; // 默认接收链接的端口号为8080
  if (argc > 1) { // 如果外部传参指定了端口号，则按照指定的端口号绑定
    // atoi()函数将数字格式的字符串转换为整数类型，需引用头文件<stdlib.h>
    port = atoi(argv[1]);
  }

  /* 开启server的一系列行动 */
  BasicTcp server(port);
  server.create_socket();
  if (server.name_socket()) {
    server.server_listen(10);
  } else {
    return -1;
  }
  
  reactor_sub_list.resize(THREAD_NUM);
  reactor_sub_list.reserve(THREAD_NUM);

#if ARRAY
  array<thread, THREAD_NUM> thread_sub_list;
#endif

  for (int i = 0; i < THREAD_NUM; ++i) {
    reactor_sub_list[i] = new ReactorMulitSub(EPOLL_CREATE_SIZE);

#if ARRAY
    thread_sub_list[i] = thread(&ReactorMulitSub::event_loop, ref(reactor_sub_list[i]),
                EPOLL_LISTEN_SIZE, EPOLL_TIMEOUT, EPOLL_MODE, call_back_handler);
#endif

    thread thread_sub_temp(&ReactorMulitSub::event_loop, ref(reactor_sub_list[i]),
                           EPOLL_LISTEN_SIZE, EPOLL_TIMEOUT, EPOLL_MODE, call_back_handler);
    thread_sub_temp.detach();
  }

  ReactorMulitMain* reactor_main = new ReactorMulitMain(EPOLL_CREATE_SIZE);
  thread thread_main = thread(&ReactorMulitMain::event_loop, ref(reactor_main),
                     EPOLL_LISTEN_SIZE, EPOLL_TIMEOUT,
                     server, EPOLL_MODE, reactor_sub_list, call_back_main);
  thread_main.detach();

#if ARRAY
  for (auto it = thread_sub_list.begin(); it != thread_sub_list.end(); it++) {
      it->detach();
  }
#endif

  while(1){}

  server.close_socket();

  return 0;
}