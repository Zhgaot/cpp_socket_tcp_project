#include "basic_tcp.h"
#include "handler.h"
#include "thread_pool.hpp"
#include "reactor_single.hpp"
#include "config.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/epoll.h> // 仅可在Linux中使用
#include <thread>

using namespace std;
using namespace TP;

/* CONFIG */
const int THREAD_NUM 
  = Config::getInstance()->conf_msg["reactor_single"]["thread_pool"]["quantity"].as<int>();
const int LISTEN_NUM
  = Config::getInstance()->conf_msg["reactor_single"]["listen"].as<int>();
const bool REPLY
  = Config::getInstance()->conf_msg["reactor_single"]["reply"].as<bool>();
const unsigned short DEFAULT_PORT
  = Config::getInstance()->conf_msg["reactor_single"]["default_port"].as<unsigned short>();
const int EPOLL_CREATE_SIZE
  = Config::getInstance()->conf_msg["reactor_single"]["epoll_create_size"].as<int>();
const int EPOLL_LISTEN_SIZE
  = Config::getInstance()->conf_msg["reactor_single"]["epoll_listen_size"].as<int>();
const int EPOLL_TIMEOUT
  = Config::getInstance()->conf_msg["reactor_single"]["epoll_timeout"].as<int>();
const string EPOLL_MODE
  = Config::getInstance()->conf_msg["reactor_single"]["epoll_mode"].as<string>();

/* 创建线程池 */
ThreadPool thread_pool(THREAD_NUM);

void call_back(const int& fd) {
  BasicTcp client;
  client.sock = fd;
  RecvSendThread* handler = new RecvSendThread(client);
  std::function<void(bool)> submit_func = std::bind(&RecvSendThread::recv_send, handler, std::placeholders::_1);
  thread_pool.submit(submit_func, REPLY);
}

/**
 * @brief 主线程入口函数
 * @param argc 传入参数的个数
 * @param argv argv[0]:程序运行的全路径名; argc[1]:传入的参数(主要用于传入端口号)
*/
int main(int argc, char *argv[]) {

  /* 确定server端口号 */
  unsigned short port = DEFAULT_PORT;
  if (argc > 1) {
    port = atoi(argv[1]);
  }

  /* 开启server的一系列行动 */
  BasicTcp server(port);
  server.create_socket();
  if (server.name_socket()) {
    server.server_listen(LISTEN_NUM);
  } else {
    return -1;
  }
  
  thread_pool.init();

  ReactorSingle my_reactor(EPOLL_CREATE_SIZE);
  my_reactor.add_event(server.sock, EPOLL_MODE);
  my_reactor.event_loop(EPOLL_LISTEN_SIZE, EPOLL_TIMEOUT, server, EPOLL_MODE, call_back);

  thread_pool.shutdown();
  server.close_socket();

  return 0;
}