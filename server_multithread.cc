#include "basic_tcp.h"
#include "handler.h"
#include "thread_pool.hpp"
#include "config.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
// #include <stdint.h>
#include <string.h>
#include <string>
#include <thread>
#include <utility>

using namespace std;
using namespace TP;

const bool THREAD_POOL 
  = Config::getInstance()->conf_msg["multithread"]["thread_pool"]["use"].as<bool>();
const int THREAD_NUM 
  = Config::getInstance()->conf_msg["multithread"]["thread_pool"]["quantity"].as<int>();
const bool REPLY
  = Config::getInstance()->conf_msg["multithread"]["reply"].as<bool>();
const int LISTEN_NUM
  = Config::getInstance()->conf_msg["multithread"]["listen"].as<int>();

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

  /* 创建线程池 */
  ThreadPool thread_pool(THREAD_NUM);
  thread_pool.init();

  BasicTcp server(port);
  server.create_socket();
  if (server.name_socket()) {
    server.server_listen(LISTEN_NUM);
  } else {
    return -1;
  }

  /* 循环接受连接(支持多个客户端同时连接到服务端)，并获取客户端的信息 */
  while (true) {
    BasicTcp client = server.accept_connection();
    /* 创建一个线程专门用于客户端与服务端之间收发数据 */
    RecvSendThread* cur_rs_thread = new RecvSendThread(
        client); // 只有当前类是new出来的，才能在对象中使用delete this;释放空间
    bool REPLY = false;

    /* // server端键盘输入是否需要手动回复client端
    while (true) {
      cout << "Please select whether you need to manually keyboard reply to the "
              "information(y/n):";
      string reply_choose;
      cin >> reply_choose;
      if (reply_choose == "y") {
        REPLY = true;
        break;
      } else if (reply_choose == "n") {
        REPLY = false;
        break;
      } else {
        cout << "Please enter (y/n)!" << endl;
        continue;
      }
    }
    */

    if (THREAD_POOL) {
      // 使用线程池
      std::function<void(bool)> submit_func = std::bind(&RecvSendThread::recv_send, cur_rs_thread, std::placeholders::_1);
      thread_pool.submit(submit_func, REPLY);
    } else {
      // 直接为每条Client连接创建线程进行处理
      thread cur_thread(&RecvSendThread::recv_send, std::ref(cur_rs_thread),
                      REPLY);
      cur_thread.detach();
    }
  }

  if (THREAD_POOL)
    thread_pool.shutdown();
  server.close_socket();

  return 0;
}
