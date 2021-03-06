#pragma once // 防止头文件重复包含

#include <string>

class BasicTcp {
public:
  BasicTcp();                      // 构造函数
  BasicTcp(unsigned short e_port); // 有参构造函数
  // ~XTcp();  // 析构函数
  int create_socket(); // 创建连接用的socket描述符
  bool name_socket();  // 命名socket(即绑定ip地址和端口号)
  bool create_connection(const char *ip, unsigned short port,
                         int timeout_ms = 1000); // 创建连接
  bool server_listen(const int listen_num);      // 服务端开启监听
  BasicTcp accept_connection();                  // 服务端接收连接
  bool set_block(bool is_block);                 // 对阻塞方式进行设置
  void close_socket();                           // 关闭socket连接
  int receive_msg(char *buf, int recv_size);     // 接收消息
  int send_msg(const char *buf, int send_size);  // 发送消息
  void byteorder(); // 用于检查机器字节序
public:
  int sock; // 连接用的socket描述符(在构造函数中初始化)
  unsigned short port; // 端口号(在构造函数中初始化)
  std::string ip;      // ip地址
};
