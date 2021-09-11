#ifdef WIN32
#include <windows.h> // 在Windows下需导入但在Linux下无需导入的头文件
#else
#include <sys/socket.h>
#include <sys/types.h> // Linux下编译需要导入的头文件
#include <unistd.h>    // close函数需要导入的头文件
#define closesocket close // Linux下关闭socket需要使用close，因此在Linux下将closesocket替换为close
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#endif
#include "basic_tcp.h"
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <thread>
#define INET_ADDRSTRLEN 16
using namespace std;

BasicTcp::BasicTcp() : sock(-1), port(0) {
  /* 为了兼容windows下的使用，在BasicTcp类的构造函数内初始化动态链接库 */
#ifdef WIN32                // 在Linux中无需初始化动态链接库
  static bool first = true; // 首次初始化为true，也就是需要进行初始化
  if (first == true) {
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws); // 2,2表示版本号
    first = false;
  }
#endif
}

BasicTcp::BasicTcp(unsigned short e_port) : sock(-1), port(e_port) {
  /* 为了兼容windows下的使用，在XTcp类的构造函数内初始化动态链接库 */
#ifdef WIN32                // 在Linux中无需初始化动态链接库
  static bool first = true; // 首次初始化为true，也就是需要进行初始化
  if (first == true) {
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws); // 2,2表示版本号
    first = false;
  }
#endif
}

int BasicTcp::create_socket() {
  /* 创建socket描述符(这个socket描述符是服务端用来接收连接的)，并作失败判断 */
  // int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //
  // 使用IPPROTO_TCP在Linux中编译不通过
  this->sock =
      socket(PF_INET, SOCK_STREAM,
             0); // PF_INET(IPv4网络协议中采用的协议族)，SOCK_STREAM(TCP/IP协议)
  if (this->sock == -1) {
    cout << "Create socket failed!" << endl;
    return -1;
  } else
    cout << "The file descriptor of socket is: " << this->sock
         << endl; // 输出socket描述符
}

bool BasicTcp::name_socket() {
  /* 万一外部没有创建socket，在这里也创建上 */
  if (this->sock == -1)
    this->create_socket();

  /* 使用TCP/IP协议的<专用socket地址>来设置地址族、端口号、IP地址 */
  sockaddr_in saddr;
  saddr.sin_family =
      AF_INET; // AF_INET是地址族，对应于IPv4网络协议中采用的协议族PF_INET
  saddr.sin_port = htons(
      this->port); // htons()函数将短整型的主机字节序数据转化为网络字节序数据，通常用于转换端口号
  saddr.sin_addr.s_addr = htons(
      0); // 设置IPv4地址(要用网络字节序表示)，设置为0表示任意ip地址发过来的数据均可接收

  /* 命名socket：即将一个socket与socket地址绑定 */
  if (bind(this->sock, (sockaddr *)&saddr, sizeof(saddr)) != 0) {
#ifdef WIN32
    cout << "[" << __FILE__ << ":" << __LINE__ << "] " 
         << "name(bind) socket failed！The port that you want to bind is: "
         << port << "; "
         << "The reason for the failure is: " << GetLastError()
         << endl; // Windows下使用GetLastError()获得错误码
#else
    cout << "[" << __FILE__ << ":" << __LINE__ << "] " 
         << "name(bind) socket failed！The port that you want to bind is: "
         << port << "; "
         << "The reason for the failure is: " << strerror(errno)
         << endl; // Linux下使用strerror解读errno来得知为何绑定失败
#endif
    return false;
  } else {
    cout
        << "name(bind) socket successfully! The port that you want to bind is: "
        << port << endl;
    return true;
  }
}

bool BasicTcp::server_listen(const int listen_num) {
  /* 监听socket */
  int listen_res = listen(
      this->sock,
      listen_num); // 第二个参数backlog表示内核监听队列的最大长度，若监听队列长度超过此值，服务器将不再受理新的客户连接
  if (listen_res == 0) { // listen成功则返回0
    cout << "The server has started listening......" << endl;
    return true;
  } else {
    cout << "Server listening failed!" << endl;
    return false;
  }
}

bool BasicTcp::create_connection(const char *ip, unsigned short port,
                                 int timeout_ms) {
  /* 万一外部没有创建socket，在这里也创建上 */
  if (this->sock == -1)
    this->create_socket();

  /* 使用TCP/IP协议的<专用socket地址>来设置地址族、端口号、IP地址 */
  sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(port);
  saddr.sin_addr.s_addr = inet_addr(ip);

  /* 创建连接，若失败则打印错误码 */
  this->set_block(false); // 改成非阻塞模式，则下面的connect将立即返回
  fd_set fdset; // 包含一个整型数组，其中每个元素的每一位标记一个文件描述符
  if (connect(sock, (sockaddr *)&saddr, sizeof(saddr)) != 0) {
    FD_ZERO(&fdset);            // 清除fdset的所有位
    FD_SET(this->sock, &fdset); // 设置fdset的位this->sock
    timeval tm;
    tm.tv_sec = 0;
    tm.tv_usec = timeout_ms * 1000; // 微秒*1000=毫秒
    if (select(this->sock + 1, 0, &fdset, 0, &tm) <= 0) {
      cout << "[" << __FILE__ << ":" << __LINE__ << "]\n" 
           << "Connect timeout or error!\n" // 超时或失败时select返回-1
           << "Failed to connect other side!\n"
           << "ip: " << ip << ", port: " << port << "\n"
           << "error: " << strerror(errno) << endl;
      this->set_block(true); // 连接失败，重设回阻塞模式
      return false;
    }
  }
  this->set_block(true); // 连接成功，但后续要进行收发数据，因此设置回阻塞模式
  cout << "Connect success!" << endl;
  return true;
}

BasicTcp BasicTcp::accept_connection() {
  /**
   * @return 返回的是一个从server角度看到的client，它包含有socket、ip、port
   */
  BasicTcp server_client; // 创建一个XTcp对象，无论最终是否accept成功，都将其返回，外部可以通过XTcp对象内的sock进行判断是否accept成功
  sockaddr_in caddr; // 该socket地址用于存储客户端的信息
  socklen_t len = sizeof(caddr);
  int client_sock = accept(sock, (sockaddr *)&caddr, &len);
  // int client_sock = accept(sock, (sockaddr*)&caddr, sizeof(caddr));
  if (client_sock < 0) { // 接受客户端连接失败
    cout << "[" << __FILE__ << ":" << __LINE__ << "] " 
         << "Failed to accept client connection，the reason for the failure is: "
         << strerror(errno) << endl;
  } else { // 接受客户端连接成功
    char remote[INET_ADDRSTRLEN];
    server_client.sock = client_sock; // 将专门用于与客户端通信的socket描述符传入进去
    server_client.ip =
        inet_ntop(AF_INET, &caddr.sin_addr, remote,
                  INET_ADDRSTRLEN); //《Linux高性能服务器编程》: p74 & p79
                                    // 另：const char* == std::string
    server_client.port = ntohs(
        caddr.sin_port); // 获取端口号(unsigned short表明了端口号最多为65535)
    cout << "connection succeeded！\n"
         << "The ip of the client is: " << server_client.ip << "\n"
         << "The port of the client is: " << server_client.port << endl;
  }
  return server_client;
}

bool BasicTcp::set_block(bool is_block) {
  /* 设置阻塞的前提是socket文件描述符已经被创建 */
  if (this->sock == -1)
    return false;

  /* 对阻塞进行设置 */
#ifdef WIN32
  unsigned long ul = 0; // 若此值为0，表明为阻塞模式，否则为非阻塞模式
  if (!is_block) // 若用户希望当前设置成非阻塞模式
    ul = 1;      // 则ul设置为1
  ioctlsocket(this->sock, FIONBIO, &ul);
#else
  int flags = fcntl(this->sock, F_GETFL, 0);
  if (flags < 0)
    return false;
  if (is_block) {
    // O_NONBLOCK取反后：非阻塞这一位变成0，其余位全部变成1
    // O_NONBLOCK再与flags进行与操作，则只有非阻塞这一位变成0，即变为阻塞模式
    flags = flags & ~O_NONBLOCK;
  } else {
    // O_NONBLOCK的原来的值就是非阻塞这一位是1表明非阻塞，其余位均是0
    // O_NONBLOCK与flags进行或操作，将非阻塞这一位变成1，即变为非阻塞模式
    flags = flags | O_NONBLOCK;
  }
  if (fcntl(this->sock, F_SETFL, flags) != 0)
    return false;
  return true;
#endif
}

void BasicTcp::close_socket() {
  /* 在所有操作后再关闭socket */
  if (this->sock == -1)
    return;
  closesocket(
      this->sock); // closesocket是Windows下的语句，顶部的define宏定义了linux下关闭socket的语句，即：close
}

int BasicTcp::receive_msg(char *buf, int recv_size) {
  return recv(this->sock, buf, recv_size, 0);
}

int BasicTcp::send_msg(const char *buf, int send_size) {
  /**
   * @brief
   * 在send_msg中，外部指定了buf有多大(send_size)，就必须要保证发送多少的数据
   * @param buf 外部传入的数据必须不可在函数内部修改，因此指定为const类型
   */
  int sended_size = 0; // 已发送的数据的大小
  while (sended_size != send_size) {
    int len = send(this->sock, buf + sended_size, send_size - sended_size,
                   0); // 当前发送的起点：buf +
                       // sended_size；当前需要发送的大小send_size - sended_size
    if (len <= 0) {
      cout << "[" << __FILE__ << ":" << __LINE__ << "] " 
           << "Failed to send data!" << endl;
      break;
    }
    sended_size += len;
  }
  return sended_size;
}

void BasicTcp::byteorder() {
  /* 用于检查机器的字节序 */
  union {
    short value;
    char union_bytes[sizeof(short)];
  } test;
  test.value = 0x0102;
  if ((test.union_bytes[0] == 1) && (test.union_bytes[1] == 2)) {
    cout << "big endian" << endl;
  } else if ((test.union_bytes[0] == 2) && (test.union_bytes[1] == 1)) {
    cout << "little endian" << endl;
  } else {
    cout << "unknow..." << endl;
  }
}
