#include "basic_tcp.h"
#include "handler.h"
#include "thread_pool.hpp"
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
const bool THREAD_POOL 
  = Config::getInstance()->conf_msg["epoll"]["thread_pool"]["use"].as<bool>();
const int THREAD_NUM 
  = Config::getInstance()->conf_msg["epoll"]["thread_pool"]["quantity"].as<int>();
const int LISTEN_NUM
  = Config::getInstance()->conf_msg["epoll"]["listen"].as<int>();
const bool REPLY
  = Config::getInstance()->conf_msg["epoll"]["reply"].as<bool>();
const unsigned short DEFAULT_PORT
  = Config::getInstance()->conf_msg["reactor_single"]["default_port"].as<unsigned short>();
const int EPOLL_CREATE_SIZE
  = Config::getInstance()->conf_msg["epoll"]["epoll_create_size"].as<int>();
const int EPOLL_LISTEN_SIZE
  = Config::getInstance()->conf_msg["epoll"]["epoll_listen_size"].as<int>();
const int EPOLL_TIMEOUT
  = Config::getInstance()->conf_msg["epoll"]["epoll_timeout"].as<int>();
const string EPOLL_MODE
  = Config::getInstance()->conf_msg["epoll"]["epoll_mode"].as<string>();

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

  /* 创建线程池 */
  ThreadPool thread_pool(THREAD_NUM);
  thread_pool.init();

  /* 开启server的一系列行动 */
  BasicTcp server(port);
  server.create_socket();
  if (server.name_socket()) {
    server.server_listen(LISTEN_NUM);
  } else {
    return -1;
  }
  
  /* epoll */
  // 创建一个epoll描述符，它标识了内核中的一个事件表，其中最多放置256个套接字
  int epoll_fd = epoll_create(EPOLL_CREATE_SIZE);
  epoll_event ev;           // 指定事件
  ev.data.fd = server.sock; // 首先，希望epoll监听server等待连接的socket
  ev.events = EPOLLIN | EPOLLET; // 采用ET(边沿触发)模式
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server.sock,
            &ev); // 操作epoll的内核事件表：新增1个socket到epoll中

  /* 循环接受连接(支持多个客户端同时连接到服务端)，并获取客户端的信息 */
  epoll_event events_out[EPOLL_LISTEN_SIZE]; // 就绪事件输出列表，最多20个事件，满了就返回
  while (true) { // 循环调用epoll_wait来获取被监测的文件描述符的信息
    // epoll_wait在一段超时时间内等待一组文件描述符上的事件
    int count = epoll_wait(epoll_fd, events_out,
                           EPOLL_LISTEN_SIZE, EPOLL_TIMEOUT);
    if (count <= 0) // epoll_wait调用失败(-1)或未等待到任何一个事件(0)
      continue;
    server.set_block(false);
    for (int i = 0; i < count; i++) {  // 循环处理多个事件
      if (events_out[i].data.fd == server.sock) { // 等待到了新的连接
        while (true) { // server可能同时接收到了多个连接，会变动很多次
          BasicTcp client = server.accept_connection(); // 接受新的连接
          if (client.sock == -1)
            break; // 只有当读到空，才跳出循环，免得漏掉连接
          // 每来一个连接，则将其对应的socket新增到epoll事件表中：
          ev.data.fd = client.sock;
          if (EPOLL_MODE == "ET")
            ev.events = EPOLLIN | EPOLLET;
          else
            ev.events = EPOLLIN;
          epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client.sock, &ev);
        }
      } else { // 等待到了已经连接上的socket
        BasicTcp client;
        client.sock = events_out[i].data.fd;
        RecvSendThread* temp_reply = new RecvSendThread(client);
        if (THREAD_POOL) {
          std::function<void(bool)> submit_func = std::bind(
            &RecvSendThread::recv_send, temp_reply, std::placeholders::_1);
        thread_pool.submit(submit_func, REPLY);
        } else {
          temp_reply->recv_send(REPLY);
        }
      }
    }
  }

  if (THREAD_POOL)
    thread_pool.shutdown();  // 关闭线程池
  server.close_socket();

  return 0;
}