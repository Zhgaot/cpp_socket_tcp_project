#include "basic_tcp.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/epoll.h> // 仅可在Linux中使用
#include <thread>
using namespace std;

int main(int argc, char *argv[]) {
  /**
   * @brief 主函数
   * @param argc 传入参数的个数
   * @param argv
   * 第0个参数为“程序运行的全路径名”，从第1个参数开始为传入的参数(主要用于传入端口号)
   */

  unsigned short port = 8080; // 默认端口号为8080
  if (argc > 1) {
    port = atoi(
        argv[1]); // atoi()函数将数字格式的字符串转换为整数类型，需要引用头文件<stdlib.h>
  }

  BasicTcp server(port);
  server.create_socket();
  server.name_socket();
  server.server_listen(10);

  /* epoll */
  // 创建一个epoll描述符，它标识了内核中的一个事件表，其中最多放置256个套接字
  int epoll_fd = epoll_create(256);
  epoll_event ev; // 指定事件
  ev.data.fd = server.sock;
  ev.events = EPOLLIN | EPOLLET; // 采用ET(边沿触发)模式
  epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server.sock,
            &ev); // 操作epoll的内核事件表：新增1个socket到epoll中
  epoll_event events[20]; // 最多20个事件，满了就返回

  /* 循环接受连接(支持多个客户端同时连接到服务端)，并获取客户端的信息 */
  char recv_buf[1024];
  char *send_msg = "HTTP/1.1 200 OK \r\n\r\n";
  int size = strlen(send_msg);
  while (true) { // 循环调用epoll_wait来获取被监测的文件描述符的信息
    // epoll_wait在一段超时时间内等待一组文件描述符上的事件
    int count = epoll_wait(epoll_fd, events, 20, 500);
    if (count <= 0) // epoll_wait调用失败(-1)或未等待到任何一个事件(0)
      continue;
    server.set_block(false);
    for (int i = 0; i < count; i++) {
      if (events[i].data.fd == server.sock) { // 等待到了新的连接
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
        client.sock = events[i].data.fd;
        client.receive_msg(recv_buf, 1024);
        client.send_msg(send_msg, size);
        // 从epoll中删除此socket描述符，在DEL时无需传入ev，因为根本不会使用，因此传入一个旧的ev：
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client.sock, &ev);
        client.close_socket();
      }
    }

    server.close_socket();

    return 0;
  }
}