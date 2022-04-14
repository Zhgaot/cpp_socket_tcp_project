#include "basic_tcp.h"
#include "config.hpp"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

using namespace std;

/* CONFIG */
const bool REPLY
  = Config::getInstance()->conf_msg["client"]["reply"].as<bool>();
const int SENDING_TIMERS
  = Config::getInstance()->conf_msg["client"]["sending_times"].as<int>();
const long SENDING_INTERVAL
  = Config::getInstance()->conf_msg["client"]["sending_interval"].as<long>();
const unsigned short DEFAULT_PORT
  = Config::getInstance()->conf_msg["client"]["default_port"].as<unsigned short>();
const long CONNECT_TIMEOUT
  = Config::getInstance()->conf_msg["client"]["connect_timeout"].as<long>();
const long RECV_BUF_LEN
  = Config::getInstance()->conf_msg["client"]["recv_buf_len"].as<long>();
const long SEND_BUF_LEN
  = Config::getInstance()->conf_msg["client"]["send_buf_len"].as<long>();

static BasicTcp client;
static struct itimerval timer;
// static struct sigaction send_register;
void init_timer();
void delayed_send_handler();
void init_register();

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
  // BasicTcp client;
  client.create_socket();
  client.set_block(false);
  client.create_connection(ip.c_str(), port, CONNECT_TIMEOUT);

  if (REPLY) {
    for (int i = 0; i < SENDING_TIMERS; ++i) {
      string send_msg_input = "";
      cout << "Please enter the reply information: ";
      cin >> send_msg_input;
      char send_msg[
        SEND_BUF_LEN > send_msg_input.size() ? SEND_BUF_LEN : send_msg_input.size()
      ];
      strcpy(send_msg, send_msg_input.c_str());
      int send_size = strlen(send_msg);
      int send_len = client.send_msg(send_msg, send_size);
      char recv_buf[RECV_BUF_LEN];
      client.receive_msg(recv_buf, sizeof(recv_buf));
      cout << "server: " << recv_buf << endl;
    }
  } else {
    init_timer();
    // init_register();
    struct sigaction send_register;
    send_register.sa_handler = (__sighandler_t)delayed_send_handler;
    send_register.sa_flags = 0;
    sigemptyset(&send_register.sa_mask);
    sigaction(SIGALRM, &send_register, NULL);
    setitimer(ITIMER_REAL, &timer, NULL);
    // delayed_send_handler();
    while(1) {}
  }
  return 0;
}

void init_timer() {
  timer.it_value.tv_sec = 1;
  timer.it_value.tv_usec = 0;  // SENDING_INTERVAL
  timer.it_interval.tv_sec = 1;
  timer.it_interval.tv_usec = 0;
}

void delayed_send_handler() {
  cout << "---------- delayed_send_handler ----------" << endl;
  client.send_msg("This is client...", 18);
  char recv_buf[RECV_BUF_LEN];
  client.receive_msg(recv_buf, sizeof(recv_buf));
  cout << "server: " << recv_buf << endl;
  /**
   * Generate timing signal:
   * - ITIMER_REAL   : SIGALRM
   * - ITIMER_VIRTUAL: SIGVTALRM
   * - ITIMER_PROF   : SIGPROF
   */
  // setitimer(ITIMER_REAL, &timer, NULL);
}

// void init_register() {
//   send_register.sa_handler = (__sighandler_t)delayed_send_handler;
//   send_register.sa_flags = 0;
//   sigemptyset(&send_register.sa_mask);
//   sigaction(SIGALRM, &send_register, NULL);
// }
