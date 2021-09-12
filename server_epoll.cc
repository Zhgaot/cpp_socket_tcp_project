#include "basic_tcp.h"
#include "reply.h"
#include "thread_pool.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/epoll.h> // 仅可在Linux中使用
#include <thread>

#define THREAD_POOL 1 // 是否使用线程池
#if THREAD_POOL
#define THREAD_NUM 3  // 线程池内线程数量
#endif

using namespace std;
using namespace TP;

int main(int argc, char *argv[]) {
  /**
   * @brief 主函数
   * @param argc 传入参数的个数
   * @param argv
   * 第0个参数为“程序运行的全路径名”，从第1个参数开始为传入的参数(主要用于传入端口号)
   */

  /* 确定server端口号 */
  unsigned short port = 8080; // 默认接收链接的端口号为8080
  if (argc > 1) { // 如果外部传参指定了端口号，则按照指定的端口号绑定
    // atoi()函数将数字格式的字符串转换为整数类型，需引用头文件<stdlib.h>
    port = atoi(argv[1]);
  }

#if THREAD_POOL
  /* 创建线程池 */
  ThreadPool thread_pool(THREAD_NUM);
  thread_pool.init();
#endif

  /* 开启server的一系列行动 */
  BasicTcp server(port);
  server.create_socket();
  if (server.name_socket()) {
    server.server_listen(10);
  } else {
    return -1;
  }
  
  /* epoll */
  // 创建一个epoll描述符，它标识了内核中的一个事件表，其中最多放置256个套接字
  int epoll_fd = epoll_create(256);
  epoll_event ev;           // 指定事件
  ev.data.fd = server.sock; // 首先，希望epoll监听server等待连接的socket
  ev.events = EPOLLIN | EPOLLET; // 采用ET(边沿触发)模式
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server.sock,
            &ev); // 操作epoll的内核事件表：新增1个socket到epoll中

  /* 循环接受连接(支持多个客户端同时连接到服务端)，并获取客户端的信息 */
  epoll_event events_out[20]; // 就绪时间输出列表，最多20个事件，满了就返回
  while (true) { // 循环调用epoll_wait来获取被监测的文件描述符的信息
    // epoll_wait在一段超时时间内等待一组文件描述符上的事件
    int count = epoll_wait(epoll_fd, events_out, 20, 10);
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
          ev.events = EPOLLIN | EPOLLET;
          epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client.sock, &ev);
        }
      } else { // 等待到了已经连接上的socket
        BasicTcp client;
        client.sock = events_out[i].data.fd;
        RecvSendThread* temp_reply = new RecvSendThread(client);
#if THREAD_POOL
        std::function<void(bool)> submit_func = std::bind(&RecvSendThread::recv_send, temp_reply, std::placeholders::_1);
        thread_pool.submit(submit_func, false);
#else
        temp_reply->recv_send(false);
#endif
      }
    }
  }

  server.close_socket();

  return 0;
}