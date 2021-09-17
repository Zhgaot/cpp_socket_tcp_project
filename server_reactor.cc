#include "basic_tcp.h"
#include "handler.h"
#include "thread_pool.hpp"
#include "reactor_epoll.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/epoll.h> // 仅可在Linux中使用
#include <thread>

#define THREAD_POOL 1 // 是否使用线程池
#if THREAD_POOL
#define THREAD_NUM 10  // 线程池内线程数量
#endif

using namespace std;
using namespace TP;

#if THREAD_POOL
  /* 创建线程池 */
  ThreadPool thread_pool(THREAD_NUM);
#endif

void call_back(RecvSendThread* handler) {
  std::function<void(bool)> submit_func = std::bind(&RecvSendThread::recv_send, handler, std::placeholders::_1);
  thread_pool.submit(submit_func, false);
}

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

  /* 开启server的一系列行动 */
  BasicTcp server(port);
  server.create_socket();
  if (server.name_socket()) {
    server.server_listen(10);
  } else {
    return -1;
  }
  
  thread_pool.init();

  Reactor my_reactor(256);
  my_reactor.add_event(server.sock, "ET");
  my_reactor.event_loop(20, 10, 10, server, "ET", call_back);

#if THREAD_POOL
  thread_pool.shutdown();
#endif
  server.close_socket();

  return 0;
}