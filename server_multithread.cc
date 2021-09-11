#include "basic_tcp.h"
#include "reply.h"
#include "thread_pool.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <thread>
#include <utility>

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

  unsigned short port = 8080; // 默认端口号为8080
  if (argc > 1) {
    port = atoi(
        argv[1]); // atoi()函数将数字格式的字符串转换为整数类型，需要引用头文件<stdlib.h>
  }

#if THREAD_POOL
  /* 创建线程池 */
  ThreadPool thread_pool(THREAD_NUM);
  thread_pool.init();
#endif

  BasicTcp server(port);
  server.create_socket();
  if (server.name_socket()) {
    server.server_listen(10);
  } else {
    return -1;
  }

  /* 循环接受连接(支持多个客户端同时连接到服务端)，并获取客户端的信息 */
  while (true) {
    BasicTcp client = server.accept_connection();
    /* 创建一个线程专门用于客户端与服务端之间收发数据 */
    RecvSendThread* cur_rs_thread = new RecvSendThread(
        client); // 只有当前线程是new出来的，才能在类中使用delete this;释放空间
    bool reply = false;
    while (true) {
      cout << "Please select whether you need to manually keyboard reply to the "
              "information(y/n):";
      string reply_choose;
      cin >> reply_choose;
      if (reply_choose == "y") {
        reply = true;
        break;
      } else if (reply_choose == "n") {
        reply = false;
        break;
      } else {
        cout << "Please enter (y/n)!" << endl;
        continue;
      }
    }
#if THREAD_POOL
    std::function<void(bool)> submit_func = std::bind(&RecvSendThread::recv_send, cur_rs_thread, std::placeholders::_1);
    thread_pool.submit(submit_func, reply);
#else
    thread cur_thread(&RecvSendThread::recv_send, std::ref(cur_rs_thread),
                      reply);
    cur_thread.detach();
#endif
  }

  thread_pool.shutdown();
  server.close_socket();

  return 0;
}
