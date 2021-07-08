#include "basic_tcp.h"
#include <iostream>
#include <stdlib.h>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
  string ip;
  unsigned short port = 8080; // 默认端口号为8080
  if (argc > 2) {
    ip = argv[1];
    port = atoi(argv[2]);
  }
  BasicTcp client;
  client.create_socket();
  client.create_connection(ip.c_str(), port);
}
